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
scheduler = new Scheduler("WEB_REQUEST", 1);

  // set task generator
 Taskgen T = Taskgen(scheduler); 
  // this will be read from file
  Worker *w1 = new Worker(1,scheduler);
  w1->startWorker();
  workers.push_front(w1);
  Worker *w2 = new Worker(2,scheduler);
  if (w2->startWorker()) 
    cout << "Started worker 2" << endl;
  workers.push_front(w2);
 tasklist=T.create_task(&task1, job1); 
  list<Task >::iterator i;
  i=tasklist.begin();

  bool tg_stop = false;
  scheduler->submitWorkers(workers);
  while (true) {
    if (stopping || currentTime == 50)
      break;
    
    /*Copied from Nav's clock function*/    
    
    //    counter ++;
    if(tg_stop == false) {
    cout<<" Time ["<<currentTime<<"] ";
    start_pos=T.add_job_list(&(*i), RATE, start_pos);
    T.send_task();
    if (start_pos == 0)
      ++i;
    if(i==tasklist.end())
      tg_stop = true;
    }
    /* Nav clock code ends here  */

    debug("Executing");

     
    // execute tg + scheduler

    // iterate all workers
    list<Worker *>::iterator worker;
    for (worker = workers.begin(); worker != workers.end(); ++worker) {
      (*worker)->execute();
    }
    
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

void Simulator::debug(const char* msg) {
  if (DEBUG)
    cout << "[Simulator][" << currentTime << "] " << msg << "\n";
}
