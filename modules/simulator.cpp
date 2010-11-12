#include <list>
#include <iostream>

#include "simulator.h"

using namespace std;
long currentTime =0;

 Simulator::Simulator() {
  stopping = false;
  start_pos=0;
  logger = Logger::getLogger();

}


void Simulator::execute() {
  Job job1[NO_JOBS];
  Task task1;
  Scheduler *scheduler;
  scheduler = new Scheduler();
  
  // set task generator
  Taskgen task_generator = Taskgen(scheduler); 
  
  // read list of workers
  readWorkers(scheduler);

  scheduler->submitWorkers(workers);

  tasklist = task_generator.create_task(&task1, job1); 
  list<Task >::iterator current_task;
  current_task=tasklist.begin();
  
  bool task_generator_stop = false;

  while (true) {
    if (scheduler->areAllJobsCompleted() && task_generator_stop)
      break;

    // run task generator
    if(task_generator_stop == false) {
      start_pos = task_generator.add_job_list(&(*current_task), RATE, start_pos);
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

  Worker *w2 = new Worker(2,scheduler);
  w2->startWorker();
  workers.push_back(w2);

  Worker *w3 = new Worker(3,scheduler);
  w3->startWorker();
  workers.push_back(w3);
  /*
  Worker *w4 = new Worker(4,scheduler);
  w4->startWorker();
  workers.push_back(w4);
  */


  return true;
}

void Simulator::logRunningAverage() {
  int no_workers = 0;
  int queued_jobs = 0;
  list<Worker *>::iterator worker;
  for (worker = workers.begin(); worker != workers.end(); ++worker) {
    if ((*worker)->ping()) {
      no_workers++;
      queued_jobs += (*worker)->getQueuedJobs();
    }
  }
  logger->workerAverage(currentTime, no_workers, queued_jobs);
}

void Simulator::debug(const char* msg) {
  if (DEBUG)
    cout << "[Simulator][" << currentTime << "] " << msg << "\n";
}
