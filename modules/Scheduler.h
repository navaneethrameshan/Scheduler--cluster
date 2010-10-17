#include <iostream>
#include <string>
#include <cstdlib>
#include <list>
#include "worker.h"
using namespace std;
class Scheduler {

 private:
  string scheduler_mode; //"SINGLE_TASK" or "WEB"
  unsigned int scheduling_interval; //time between executions of the scheduling algorithm
  list<Worker *> workers;    //Contains information of all worker nodes
  list<Job *> queuedJobs;    //Contains jobs which the Scheduler has received and have not started running
  list<Job *> completedJobs; //Contains jobs which have been completed
  list<Job *> runningJobs;   //Contains jobs which are currently in running state
  

  
 public:


  // default constructor
  Scheduler();
  
  // constructor
  Scheduler(string scheduler_mode, unsigned int scheduling_interval, list<Worker *> workers);

  //this function will start a worker and return its worker_id
  unsigned int startWorkerNode();
  
  //this function will stop a worker and returns 0 on successful execution
  unsigned int stopWorkerNode(unsigned int worker_id);  
  
  // TaskGenerator will submit jobs to the scheduler using this function
  int submitJobs(list<Job *> jobs);

  // if required, the Simulator can submit new worker nodes to Scheduler 
  int submitWorkers(list<Worker *> workers);

  //Runs the scheduler - will be executed at each clock tick by Simulator
  int runScheduler(); 
    
  // a Worker node will notify the Scheduler when a job finishes its execution
  int notifyJobCompletion(int job_id);

  // outputs the current state of a Scheduler object (can be static also; will be decided later on)
  void print();
};
