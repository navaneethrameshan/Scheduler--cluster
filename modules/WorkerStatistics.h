/*! \brief A class to store Worker Statistics which are used by the Scheduler to makescheduling decisions. The attributes of the objects of this class are periodically updated by the scheduler during the simulation.
 */

#ifndef __WORKERSTATISTICS__
#define __WORKERSTATISTICS__


#include <iostream>
#include <string>
#include <cstdlib>
#include <list>
#include <math.h>
using namespace std;

#include "worker.h"
#include "logger.h"

class WorkerStatistics {

 private:
  int worker_id;
  double average_response_time;
  double available_memory;
  int number_of_running_jobs;
  int number_of_queued_jobs;
  int number_of_submitted_jobs;
  int number_of_total_submitted_jobs;
  double worker_duration_till_now;
  double worker_cost_till_now;
  long created_time;
  long last_updated;
  Logger* log;

 public:

  // default constructor
  WorkerStatistics(int workerid, long createdtime);
  
  //getter functions for all attributes 
  int getWorkerID();
  double getAverageResponseTime();
  double getAvailableMemory();
  int getNumberOfRunningJobs();
  int getNumberOfQueuedJobs();
  int getNumberOfSubmittedJobs();
  int getNumberOfTotalSubmittedJobs();
  double getWorkerDurationTillNow();
  double getWorkerCostTillNow();
  long getCreatedTime();
  long getLastUpdatedTime();
  
  //increment functions
  void incrementSubmittedJobs(long currenttime);
  void incrementQueuedJobs(long currenttime);  
  void incrementRunningJobs(long currenttime);
  void incrementTotalSubmittedJobs(long currenttime);
  
  //decrement functions
  void decrementSubmittedJobs(long currenttime);
  void decrementRunningJobs(long currenttime);
  void decrementQueuedJobs(long currenttime);
  

  //setter functions for all attributes 
  void setWorkerID(int workerid, long currenttime);
  void setAverageResponseTime(double avgresptime, long currenttime);
  void setAvailableMemory(double avmem, long currenttime);
  void setNumberOfRunningJobs(int numrunningjobs, long currenttime);
  void setNumberOfQueuedJobs(int numqueuedjobs, long currenttime);
  void setNumberOfSubmittedJobs(int numsubmittedjobs, long currenttime);
  void setNumberOfTotalSubmittedJobs(int numtotalsubmittedjobs, long currenttime);
  void setWorkerDurationTillNow(double workerduration, long currenttime);
  void setWorkerCostTillNow(double workercost, long currenttime);
  void setCreatedTime(long createdtime); //not required - should be deprecated
  void setLastUpdatedTime(long lastupdated); //not required - should be deprecated

  
  // outputs the current state of the object 
  void print();

  //TODO: number of submitted jobs
};


#endif /* ____ */
