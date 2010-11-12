#ifndef __JOB__
#define __JOB__

class Job {
  int task_id;	
  int no_inst;
  int mem_size;

  int instructions_completed;
  
 public:
  int job_id;
  
  void init(int taskid, int jobid, int inst, int mem);
  void show();
  //added by wasif
  unsigned int getJobID();
  // added by archie
  int getNumberOfInstructions();
  int getMemoryConsumption();
  bool addInstructionsCompleted(int instructions);
  int getInstructionsCompleted();
};

#endif /* __WORKER__ */
