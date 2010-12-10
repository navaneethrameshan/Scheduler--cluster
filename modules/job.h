#ifndef __JOB__
#define __JOB__

#include <string>

using namespace std;

class Job {
  int task_id;	
  int no_inst;
  int mem_size;

  int instructions_completed;
  int swap_count;
  unsigned long started;
  
 public:
  int job_id;
  
  void init(int taskid, int jobid, int inst, int mem);
  string show();
  //added by wasif
  unsigned int getJobID();
  // added by archie
  int getNumberOfInstructions();
  int getMemoryConsumption();
  int addInstructionsCompleted(int instructions);
  int getInstructionsCompleted();
  int increaseSwapCount();
  unsigned long getStartedTime();
  void setStartedTime();
};

static inline bool 
operator == (const Job &a, const Job &b) {
  return (a.job_id == b.job_id);
}


#endif /* __WORKER__ */
