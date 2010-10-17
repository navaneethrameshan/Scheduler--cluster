#ifndef __WORKER__
#define __WORKER__

#include "job.h"

#ifndef DEBUG
#define DEBUG true
#endif

extern long currentTime;

enum worker_states { INITIALISING, COMPUTING, FINALISING, IDLE, OFFLINE };

struct WORKER_PROPERTIES {
  int memory;
  float cost_per_hour;
  long time_to_startup;
  long swapping_time; 
  long instructions_per_time;
};

struct WORKER_STATE {
  enum worker_states current;
  long start;
  long started;
  long time_spent; //unused
  int available_memory;
  bool accepting_jobs;
};

class Worker {

 private:
  int id;
  WORKER_STATE state;
  WORKER_PROPERTIES properties;
  Job *current_job;
  std::list<Job> jobs;

  void initialise();
  void compute();
  void finalise();
  bool setState(enum worker_states newstate);
  bool setState(enum worker_states newstate, bool accept_jobs);
  long getTotalComputationTime();
  void debug(const char *msg);

 public:
  Worker(int id);
  void execute();
  bool startWorker();
  bool stopWorker();
  bool submitJobs(std::list<Job> newjobs);
  enum worker_states getState();
  int getAvailableMemory();
  bool getAcceptingJobs();
  int getTotalMemory();
  float getCostPerHour();
  long getTimeToStart();
  long getSwappingTime();
  long getInstructionsPerTime();

//added by wasif
unsigned int getWorkerID();

};

#endif /* __WORKER__ */
