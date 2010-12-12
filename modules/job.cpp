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
	started = 0;
	endtime = 0;
	starttime =0;
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

unsigned int Job::getTaskID() {
	return task_id;
}

//added by archie
int Job::getNumberOfInstructions() {
  return no_inst;
}

int Job::getMemoryConsumption() {
  return mem_size;
}

float Job::addInstructionsCompleted(float instructions) {
  instructions_completed += instructions;
  if (instructions_completed > no_inst) 
    return instructions_completed-no_inst;

  return 0;
}

float Job::getInstructionsCompleted() {
  return instructions_completed;
}

int Job::increaseSwapCount() {
  return swap_count++;
}

void Job::setStartedTime() {
  started = currentTime;
}

void Job::setStartTime() {
	starttime = currentTime;
}

void Job::setEndTime() {
	endtime = currentTime;
}

unsigned long Job::getEndTime() {
	return endtime;
}

unsigned long Job::getStartedTime() {
  return started;
}
unsigned long Job::getStartTime() {
	return starttime;
}
