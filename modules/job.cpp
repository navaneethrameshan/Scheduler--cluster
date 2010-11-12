#include"job.h"
#include<iostream>
using namespace std;

void Job:: init(int taskid, int jobid, int inst, int mem){
  task_id=taskid;
  job_id=jobid;
  no_inst=inst;
  mem_size=mem;
}

void Job:: show(){
  cout<< "\nTask ID: "<<task_id<< " Job ID: "<< job_id<<" Job Inst: "<<no_inst<<" Job Mem: "<<mem_size<< "\n";
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
