#include <list>
#include <iostream>
#include <fstream>

#include "simulator.h"

using namespace std;
unsigned long currentTime = 0;

 Simulator::Simulator() {
  stopping = false;
  start_pos=0;
  logger = Logger::getLogger();

}


void Simulator::execute() {
  Job job1[1000] [100];
  Task task1[1000];
  config = readSimulatorConfig();
  Scheduler *scheduler = new Scheduler(config->scheduler_mode,
                                       config->scheduling_interval,
                                       config->scheduling_interval_for_clock);
  //modified Nov-8
  int total_input = 0;
  string strings;
  fstream file("config/input.conf");
  getline(file, strings);
  
  while (file){
    total_input++;
    getline(file, strings);
  }

  
  file.close();
  //modified Nov-8
  
  // set task generator
  Taskgen task_generator = Taskgen(scheduler); 
  
  // read list of workers
  readWorkers(scheduler);

  scheduler->submitWorkers(workers);

  tasklist = task_generator.create_task(task1, job1, total_input); 
  
  list<Task >::iterator current_task;
  current_task=tasklist.begin();
  
  bool task_generator_stop = false;

  while (true) {
    if (scheduler->areAllJobsCompleted() && task_generator_stop)
      break;

    // run task generator
    if (currentTime % 1000 == 0) {
      if(task_generator_stop == false) {
	start_pos = task_generator.add_job_list(&(*current_task), 
						current_task->job_rate, 
						start_pos);
	task_generator.send_task();

	if (start_pos == 0)
	  ++current_task;

	if(current_task==tasklist.end())
	  task_generator_stop = true;

      }


    }

      runWorkers();    
    
    
    scheduler->runScheduler();
    //    scheduler->print();   //just added for debugging - can be removed
    
    if ((currentTime % 100) == 0) {
      logRunningAverage();
      scheduler->print();
    }

    currentTime++;
  }
  scheduler->print();   
  //scheduler->printSummary();
  cleanUp();
}

void Simulator::stop() {
  stopping = true;
}

bool Simulator::cleanUp() {
  logRunningAverage();
  logTotals();
  return true;
}


/* Private methods */
void Simulator::runWorkers() {
  list<Worker *>::iterator worker;

  for (worker = workers.begin(); worker != workers.end(); ++worker) {
    //    logger->debugInt("Now executing worker", (*worker)->getWorkerID());
    (*worker)->execute();
  }
}

bool Simulator::readWorkers(Scheduler *scheduler) {
  // todo: should be read from file - DONE
  ifstream infile;
  string values[20];
  string line;
  int i=0;
  infile.open ("config/workers.conf");

  while(!infile.eof()) // To get you all the lines.
    {
      getline(infile,line); // Saves the line in STRING.
      if( (line.find('#') != 0) ) //this will ignore the lines having a '#' character 
	{
	  values[i] = line;
	  i++;
	}
    }
  infile.close();

  int started_workers = strtod(values[0].c_str(), NULL);
  int workers_to_create = strtod(values[1].c_str(),NULL);

  WORKER_PROPERTIES* properties = new WORKER_PROPERTIES;
  properties->memory = (int)config->worker_node_memory * 1024;
  properties->cost_per_hour = config->worker_node_cost;
  properties->time_to_startup = (int)config->worker_node_startup_time;
  properties->swapping_time = (int)config->worker_node_swapping_cost;
  properties->instructions_per_time = (long)config->worker_node_speed;
  properties->quantum = (int)config->worker_quantum;

  for (int i = 1; i <= workers_to_create; i++) {
    Worker *worker = new Worker(i, properties, scheduler);
    if (i <= started_workers)
      worker->startWorker();
    workers.push_back(worker);
  }

  return true;
}

SIMULATOR_CONFIG* Simulator::readSimulatorConfig() {
  string line;
  ifstream infile;
  string values[10];
  int i = 0;
  SIMULATOR_CONFIG *config = new SIMULATOR_CONFIG; 

  infile.open ("config/scheduler.conf");
  
  while(!infile.eof()) {
    getline(infile,line); // Saves the line in STRING.
    if( (line.find('#') != 0) ) { 
      values[i] = line;
      i++;
    }
  }

  config->scheduler_mode = values[0];
  //0.01 to 1 seconds atoi(strNumber.c_str());
  config->scheduling_interval = strtod(values[1].c_str(), NULL); 
  config->scheduling_interval_for_clock = 
    (unsigned short int)floor((1/config->scheduling_interval)+0.5);
  //200 - 400 instructions per second
  config->worker_node_speed = strtod(values[2].c_str(), NULL); 
  //2-8 GB
  config->worker_node_memory = strtod(values[3].c_str(), NULL); 
  //2-10 instructions per GB
  config->worker_node_swapping_cost = strtod(values[4].c_str(), NULL); 
  //0.01 to 0.5 seconds
  config->worker_quantum = strtod(values[5].c_str(), NULL); 
  //120-600 seconds
  config->worker_node_startup_time = strtod(values[6].c_str(), NULL); 
  //1-5 instructions
  config->worker_node_sched_notif_time = strtod(values[7].c_str(), NULL);
  //in euros/hour
  config->worker_node_cost = strtod(values[8].c_str(), NULL);

  infile.close();

  return config;
}

void Simulator::logRunningAverage() {
  int offline_workers_count = 0;
  int idle_workers_count = 0;
  int computing_workers_count = 0;
  int queued_jobs = 0;
  double avg_response_time = 0;
  float cost_so_far = 0;

  list<Worker *>::iterator worker;
  for (worker = workers.begin(); worker != workers.end(); ++worker) {
    if ((*worker)->ping()) {
      queued_jobs += (*worker)->getQueuedJobs();
      if ((*worker)->getState() == COMPUTING) 
        queued_jobs++;
    }

    if( (*worker)->getState() == OFFLINE  ) {
      offline_workers_count++; 
      cost_so_far += (*worker)->getTotalCost();
    }
    if( (*worker)->getState() == IDLE  ) {
      idle_workers_count++;
      cost_so_far += (*worker)->getTotalCost();
      avg_response_time += (*worker)->getAverageResponseTime();
    }
    if( (*worker)->getState() == COMPUTING || 
        (*worker)->getState() == SWAPPING  ) {
      computing_workers_count++;
      cost_so_far += (*worker)->getTotalCost();
      avg_response_time += (*worker)->getAverageResponseTime();
    }
  } 
  
  avg_response_time = avg_response_time/(idle_workers_count + 
                                         computing_workers_count);

  logger->workerAverage(avg_response_time, 
                        cost_so_far,
                        offline_workers_count,
                        idle_workers_count,
                        computing_workers_count,
                        queued_jobs);
}

void Simulator::logTotals() {
  long totalExecutionTime = 0;
  long totalCPUTime = 0;
  float totalCost = 0;
  list<Worker *>::iterator worker;
  for (worker = workers.begin(); worker != workers.end(); ++worker) {
    totalExecutionTime += (*worker)->getTotalExecutionTime();
    totalCPUTime += (*worker)->getTotalCPUTime();
    totalCost += (*worker)->getTotalCost();
  }
  logger->totals(totalExecutionTime,
                 totalCPUTime,
                 totalCost);
}
