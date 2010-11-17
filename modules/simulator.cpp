#include <list>
#include <iostream>

#include "simulator.h"

using namespace std;
unsigned long currentTime = 0;

 Simulator::Simulator() {
  stopping = false;
  start_pos=0;
  logger = Logger::getLogger();

}


void Simulator::execute() {
  Job job1[100] [100];
  Task task1[100];
  Scheduler *scheduler;
  scheduler = new Scheduler();
//modified Nov-8
  int total_input =0;
  string strings;
fstream file("modules/input.conf");
getline(file, strings);
while (file){
	total_input ++;
	getline(file, strings);
}
//cout<< "\nTotal Input: "<<total_input<< " String: "<< strings<< "\n";
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
    if(task_generator_stop == false) {
      start_pos = task_generator.add_job_list(&(*current_task), current_task->job_rate, start_pos);
      task_generator.send_task();

      if (start_pos == 0)
        ++current_task;

      if(current_task==tasklist.end())
        task_generator_stop = true;
    }


    runWorkers();
    
    scheduler->runScheduler();
        scheduler->print();
    
    if ((currentTime % 1000) == 0) {
      logRunningAverage();
    }

    currentTime++;
  }

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

  for (worker = workers.begin(); worker != workers.end(); ++worker)
    (*worker)->execute();
}

bool Simulator::readWorkers(Scheduler *scheduler) {
  // todo: should be read from file
  Worker *w1 = new Worker(1,scheduler);
  w1->startWorker();
  workers.push_back(w1);
  /*
  Worker *w2 = new Worker(2,scheduler);
  w2->startWorker();
  workers.push_back(w2);

  Worker *w3 = new Worker(3,scheduler);
  w3->startWorker();
  workers.push_back(w3);

  Worker *w4 = new Worker(4,scheduler);
  w4->startWorker();
  workers.push_back(w4);
  */
  return true;
}

void Simulator::logRunningAverage() {
  int offline_workers_count = 0;
  int idle_workers_count = 0;
  int computing_workers_count = 0;
  int queued_jobs = 0;
  list<Worker *>::iterator worker;
  for (worker = workers.begin(); worker != workers.end(); ++worker) {
    if ((*worker)->ping()) {
      queued_jobs += (*worker)->getQueuedJobs();
      if ((*worker)->getState() == COMPUTING) 
        queued_jobs++;
    }

    if( (*worker)->getState() == OFFLINE  )
      offline_workers_count++;
    if( (*worker)->getState() == IDLE  )
      idle_workers_count++;
    if( (*worker)->getState() == COMPUTING || 
        (*worker)->getState() == SWAPPING  )
      computing_workers_count++;
  } 
  logger->workerAverage(offline_workers_count,
                        idle_workers_count,
                        computing_workers_count,
                        queued_jobs);
}

void Simulator::logTotals() {
  long totalExecutionTime = 0;
  long totalCPUTime = 0;
  long totalCost = 0;
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

void Simulator::debug(const char* msg) {
  if (DEBUG)
    cout << "[Simulator][" << currentTime << "] " << msg << "\n";
}
