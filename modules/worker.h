#ifndef __WORKER__
#define __WORKER__

#include "job.h"
#include "Scheduler.h"

#ifndef DEBUG
#define DEBUG true
#endif

class Scheduler;
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
  unsigned int id;
  Scheduler *scheduler;
  WORKER_STATE state;
  WORKER_PROPERTIES properties;
  Job *current_job;
  Job tmp_current_job;
  std::list<Job> jobs;
  long total_execution_time;
  long total_cpu_time;

  void initialise();
  void compute();
  void finalise();
  void idle();
  bool setState(enum worker_states newstate, bool accept_jobs);
  long getTotalComputationTime();
  void debug(const char *msg);
  void setDefaultProperties();
  void removeJob();
  bool startJob();
  bool hasMoreWork();
  void increaseExecutionTime();
  void increaseCPUTime();

 public:
  Worker(int id, Scheduler *sched);
  void execute();
  bool startWorker();
  bool stopWorker();
  bool submitJobs(std::list<Job> newjobs);
  enum worker_states getState();
  int getAvailableMemory();
  bool isAcceptingJobs();
  int getTotalMemory();
  float getCostPerHour();
  long getTimeToStart();
  long getSwappingTime();
  long getInstructionsPerTime();
  long getTotalExecutionTime();
  long getTotalCPUTime();
  long getTotalCost();
  bool ping();

//added by wasif
unsigned int getWorkerID();

};

#endif /* __WORKER__ */
