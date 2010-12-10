#include<iostream>
#include<sstream>
#include<string>
#include"job.h"

using namespace std;

extern unsigned long currentTime;

void Job:: init(int taskid, int jobid, int inst, int mem){
  task_id=taskid;
  job_id=jobid;
  no_inst=inst;
  mem_size=mem;
  instructions_completed = 0;
  swap_count = 0;
}

string Job:: show(){
  stringstream s;
  s << "Task ID: "<<task_id<< " Job ID: "<< job_id
    <<" Job Inst: "<<no_inst<<" Job Mem: "<<mem_size;
  return s.str();
}

//added by Wasif
unsigned int Job::getJobID()
{
  return job_id;
}

//added by archie
int Job::getNumberOfInstructions() {
  return no_inst;
}

int Job::getMemoryConsumption() {
  return mem_size;
}

int Job::addInstructionsCompleted(int instructions) {
  instructions_completed += instructions;
  cout << "INSTRUCTIONS COMPLETED: " << instructions_completed << endl;
  if (instructions_completed > no_inst) 
    return instructions_completed-no_inst;

  return 0;
}

int Job::getInstructionsCompleted() {
  return instructions_completed;
}

int Job::increaseSwapCount() {
  return swap_count++;
}

void Job::setStartedTime() {
  started = currentTime;
}

unsigned long Job::getStartedTime() {
  return started;
}

