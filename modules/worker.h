#ifndef __WORKER__
#define __WORKER__

#include "job.h"
#include "Scheduler.h"
#include "logger.h"
#include <map>

class Scheduler;
extern unsigned long currentTime;

enum worker_states { INITIALISING, COMPUTING, SWAPPING, FINALISING, IDLE, OFFLINE };

struct WORKER_PROPERTIES {
  int memory;
  float cost_per_hour;
  unsigned long time_to_startup;
  unsigned long swapping_time; 
  long instructions_per_time;
  int quantum;
  int notification_time;
};

struct WORKER_STATE {
  enum worker_states current;
  unsigned long start;
  unsigned long started;
  unsigned long average_response_time;
  long time_spent; //unused
  int available_memory;
  bool accepting_jobs;
  int jobs_completed; 
};

class Worker {

 private:
  Logger* logger;
  unsigned int id;
  Scheduler *scheduler;
  WORKER_STATE state;
  WORKER_PROPERTIES properties;
  Job *current_job;
  Job tmp_current_job;
  std::list<Job> ram;
  std::list<Job> jobs;
  long total_execution_time;
  long total_cpu_time;
  float job_carry_over;
  int tmp_job_size;
  unsigned int time_to_swap;
  std::map<long, int> job_completion;

  void initialise();
  void compute();
  void swap();
  void finalise();
  void idle();
  bool setState(enum worker_states newstate, bool accept_jobs);
  long getTotalComputationTime();
  void setProperties(WORKER_PROPERTIES *props);
  void removeJob();
  bool startJob();
  bool moveLargestJobFromMemoryToHdd();
  bool moveJobToMemory();
  bool activateJob();
  bool hasMoreWork();
  void increaseExecutionTime();
  void increaseCPUTime();
  bool swapJob();
  bool swapInMemory();
  unsigned int calculateSwappingTime(Job *job);
  void debugJobs();
  std::map<Job *, int> getJobFootprints();

 public:
  Worker(int id, WORKER_PROPERTIES *props, Scheduler *sched);
  void execute();
  bool startWorker();
  bool stopWorker();
  bool submitJobs(std::list<Job> newjobs);
  enum worker_states getState();
  int getAvailableMemory();
  bool isAcceptingJobs();
  int getTotalMemory();
  float getCostPerHour();
  float getInstructionsPerTime();
  long getTotalExecutionTime();
  long getTotalCPUTime();
  double getAverageResponseTime();
  float getTotalCost();
  int getQueuedJobs();
  bool ping();
  bool cancelJob(unsigned int taskId, unsigned int jobId);
  std::map<long,int> getCompletionTimes(int windowSize); 
  int getJobsCompleted();

//added by wasif
unsigned int getWorkerID();

};

#endif /* __WORKER__ */
