#include <list>
#include <iostream>

#include "simulator.h"

using namespace std;
long currentTime =0;

 Simulator::Simulator() {
  stopping = false;
  start_pos=0;

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
    if (stopping || currentTime == 50)
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

    currentTime++;
  }

  cleanUp();
}

void Simulator::stop() {
  stopping = true;
}

bool Simulator::cleanUp() {
  debug("Stopping.");
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
  workers.push_front(w1);

  Worker *w2 = new Worker(2,scheduler);
  w2->startWorker();
  workers.push_front(w2);

  return true;
}

void Simulator::debug(const char* msg) {
  if (DEBUG)
    cout << "[Simulator][" << currentTime << "] " << msg << "\n";
}
