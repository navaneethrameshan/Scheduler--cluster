#ifndef __SCHEDULER__
#define __SCHEDULER__


#include <iostream>
#include <string>
#include <cstdlib>
#include <list>
using namespace std;

#include "worker.h"
class Worker;
class Scheduler {

 private:
  string scheduler_mode;
  float scheduling_interval;
  float worker_node_speed;
  float worker_node_memory;
  float worker_node_swapping_cost;
  float worker_quantum;
  float worker_node_startup_time;
  float worker_node_sched_notif_time;
  float worker_node_cost;

  list<Worker *> workers;    //Contains information of all worker nodes
  list<Job > queuedJobs;    //Contains jobs which the Scheduler has received and have not started running
  list<Job > completedJobs; //Contains jobs which have been completed
  list<Job > runningJobs;   //Contains jobs which are currently in running state
  

  
 public:


  // default constructor
  Scheduler();
  
  // constructor
  Scheduler(string scheduler_mode, float scheduling_interval);

  //this function will start a worker and return its worker_id
  unsigned int startWorkerNode();
  
  //this function will stop a worker and returns 0 on successful execution
  unsigned int stopWorkerNode(unsigned int worker_id);  
  
  // TaskGenerator will submit jobs to the scheduler using this function
  int submitJobs(list<Job > jobs);

  // if required, the Simulator can submit new worker nodes to Scheduler 
  int submitWorkers(list<Worker *> workers);

  //Runs the scheduler - will be executed at each clock tick by Simulator
  int runScheduler(); 
    
  // a Worker node will notify the Scheduler when a job finishes its execution
  int notifyJobCompletion(unsigned int job_id);

  // outputs the current state of a Scheduler object (can be static also; will be decided later on)
  void print();
};


#endif /* ____ */
