#ifndef __JOB__
#define __JOB__

#include <string>

using namespace std;

class Job {
  int task_id;	
  int no_inst;
  int mem_size;

  float instructions_completed;
  int swap_count;
  unsigned long started;
	unsigned long endtime;
	unsigned long starttime;
  
 public:
  int job_id;
  
  void init(int taskid, int jobid, int inst, int mem);
  string show();
  //added by wasif
  unsigned int getJobID();
	unsigned int getTaskID();
  // added by archie
  int getNumberOfInstructions();
  int getMemoryConsumption();
  float addInstructionsCompleted(float instructions);
  float getInstructionsCompleted();
  int increaseSwapCount();
  unsigned long getStartedTime();
	unsigned long getStartTime();
  void setStartedTime();
	void setStartTime();
	void setEndTime();
	unsigned long getEndTime();
};

static inline bool 
operator == (const Job &a, const Job &b) {
  return (a.job_id == b.job_id);
}


#endif /* __WORKER__ */
