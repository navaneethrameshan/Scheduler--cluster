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
  cout<< "\nJob ID: "<< job_id<<" Job Inst: "<<no_inst<<" Job Mem: "<<mem_size<< "\n";
}
