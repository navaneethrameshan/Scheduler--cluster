//!@version 0.1
//!@author Was/Nav/Marc
//!@date November 9th, 2010
//!@note WorkerStatistics implementation file

/*! \brief A class to store Worker Statistics which are used by the Scheduler to make intelligent decisions
 */



//#include <iostream>
//#include <string>
//#include <cstdlib>
//#include <list>
//#include <math.h>

using namespace std;

//#include "worker.h"
//#include "logger.h"
#include "WorkerStatistics.h"

//class WorkerStatistics {

  // default constructor
  WorkerStatistics::WorkerStatistics(int workerid, long createdtime) 
  {
    worker_id = workerid;
    available_memory = -1;
    average_response_time = -1;
    number_of_running_jobs = 0;
    number_of_queued_jobs = 0;
    number_of_submitted_jobs = 0;
    worker_duration_till_now = 0;
    worker_cost_till_now = 0;
    created_time = createdtime;
    last_updated = createdtime;
    log = Logger::getLogger();

  }
  
  //getter functions for all attributes 
  int WorkerStatistics::getWorkerID() 
  {
    return worker_id;
  }

  double WorkerStatistics::getAverageResponseTime() 
  {
    return average_response_time;
  }

  double WorkerStatistics::getAvailableMemory() 
  {
    return available_memory;
  }
  int WorkerStatistics::getNumberOfRunningJobs()
  {
    return number_of_running_jobs;
  }
  int WorkerStatistics::getNumberOfQueuedJobs()
  {
    return number_of_queued_jobs;
  }
 int WorkerStatistics::getNumberOfSubmittedJobs()
  {
    return number_of_submitted_jobs;
  }

  double WorkerStatistics::getWorkerDurationTillNow()
  {
    return worker_duration_till_now;
  }
  double WorkerStatistics::getWorkerCostTillNow()
  {
    return worker_cost_till_now;
  }

long WorkerStatistics::getCreatedTime()
  {
    return created_time;
  }
  long WorkerStatistics::getLastUpdatedTime()
  {
    return last_updated;
  }
  
  //setter functions for all attributes 
  void WorkerStatistics::setWorkerID(int workerid, long currenttime)
  {
    worker_id = worker_id;
    last_updated = currenttime;
  }
  void WorkerStatistics::setAverageResponseTime(double avgresptime, long currenttime)
  {
    average_response_time = avgresptime;
    last_updated = currenttime;
  }
  void WorkerStatistics::setAvailableMemory(double avmem, long currenttime)
  {
    available_memory = avmem;
    last_updated = currenttime;
  }
  void WorkerStatistics::setNumberOfRunningJobs(int numrunningjobs, long currenttime)
  {
    number_of_running_jobs = numrunningjobs;
    last_updated = currenttime;
  }
  void WorkerStatistics::setNumberOfQueuedJobs(int numqueuedjobs, long currenttime)
  {
    number_of_queued_jobs = numqueuedjobs;
    last_updated = currenttime;
  }
void WorkerStatistics::setNumberOfSubmittedJobs(int numsubmittedjobs, long currenttime)
  {
    number_of_submitted_jobs = numsubmittedjobs;
    last_updated = currenttime;
  }
  void WorkerStatistics::setWorkerDurationTillNow(double workerduration, long currenttime)
  {
    worker_duration_till_now = workerduration;
    last_updated = currenttime;
  }
  void WorkerStatistics::setWorkerCostTillNow(double workercost, long currenttime)
  {
    worker_cost_till_now = workercost;
    last_updated = currenttime;
  }
  
  void WorkerStatistics::setCreatedTime(long createdtime)
  {
    created_time = createdtime;
    last_updated = createdtime;
  }
  
  void WorkerStatistics::setLastUpdatedTime(long lastupdated)
  {
    last_updated = lastupdated;
  }


 //increment functions
  void WorkerStatistics::incrementSubmittedJobs(long currenttime) 
  {
    number_of_submitted_jobs++;
    last_updated = currenttime;
  }

  void WorkerStatistics::incrementQueuedJobs(long currenttime)
  {
    number_of_queued_jobs++;
    last_updated = currenttime;
  }

  void WorkerStatistics::incrementRunningJobs(long currenttime)
  {
    number_of_running_jobs++;
    last_updated = currenttime;
  }

  
  //decrement functions
  void WorkerStatistics::decrementSubmittedJobs(long currenttime)
  {
    number_of_submitted_jobs--;
    last_updated = currenttime;
  }

  void WorkerStatistics::decrementRunningJobs(long currenttime)
  {
    number_of_running_jobs--;
    last_updated = currenttime;
  }

  void WorkerStatistics::decrementQueuedJobs(long currenttime)
  {
    number_of_queued_jobs--;
    last_updated = currenttime;
  }

  
  // outputs the current state of the object 
void WorkerStatistics::print()
  {
        cout<< 
      "worker_id:"<<worker_id<<"\t"<<
      "average_response_time:"<<average_response_time<<"\t"<<
      "available_memory:"<<available_memory<<"\t"<<
      "num_running_jobs:"<<number_of_running_jobs<<"\t"<<
      "num_queued_jobs:"<<number_of_queued_jobs<<"\t"<<
      "num_submitted_jobs:"<<number_of_running_jobs<<"\t"<<
      "worker_duration_till_now:"<<worker_duration_till_now<<"\t"<<
      "worker_cost_till_now:"<<worker_cost_till_now<<"\t"<<
      "created_time:"<<created_time<<"\t"<<
      "last_updated:"<<last_updated<<"\t"<<
        endl ;
    
    //  cout<<"Ola"<<endl;
  }
//};


//#endif /* ____ */
