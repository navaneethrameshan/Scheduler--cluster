#include"job.h"
#include<iostream>
using namespace std;

void Job:: init(int taskid, int jobid, int inst, int mem){
  task_id=taskid;
  job_id=jobid;
  no_inst=inst;
  mem_size=mem;
  instructions_completed = 0;
}

void Job:: show(){
  cout<< "\nJob ID: "<< job_id<<" Job Inst: "<<no_inst<<" Job Mem: "<<mem_size<< "\n";
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

bool Job::addInstructionsCompleted(int instructions) {
  if (instructions_completed+instructions >= no_inst)
    return false;

  instructions_completed += instructions;

  return true;
}

int Job::getInstructionsCompleted() {
  return instructions_completed;
}
