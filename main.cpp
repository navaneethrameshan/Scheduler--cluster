#include<iostream>
#include<cstdlib>
#include<list>
#include<time.h>

#include"task.h"
#include"simulator.h"
#include"worker.h"

using namespace std;

#define NO_JOBS 3
#define JOB_START_ID 0
#define TASK_ID 1
#define LOW_INST_BOUND 1000
#define HIGH_INST_BOUND 1000
#define INST LOW_INST_BOUND+rand()%((HIGH_INST_BOUND-LOW_INST_BOUND!=0)?(HIGH_INST_BOUND-LOW_INST_BOUND):1)
#define LOW_MEM 512
#define HIGH_MEM 1024
#define MEM LOW_MEM+rand()%((HIGH_MEM-LOW_MEM!=0)?(HIGH_MEM-LOW_MEM):1)

int main(){
  srand(time(NULL));
  job job1[NO_JOBS];
  for(int i=0;i<NO_JOBS;i++){
    job1[i].init((i+JOB_START_ID),INST, MEM);
  }
  Task task1;
  task1.init(TASK_ID,NO_JOBS,job1);
  //Rate  of job generation needs to be considered as well.
  task1.show();
  
  Simulator simulator;
  simulator.init();
  simulator.execute();
  
  return 0;
}
        

