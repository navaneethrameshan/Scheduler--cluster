//!@version 0.1
//!@author Was/Nav/Marc
//!@date November 9th, 2010
//!@note Scheduler header file

/*! \brief A chess piece
 * The abstract parent of all chess pieces. */

#ifndef __SCHEDULER__
#define __SCHEDULER__


#include <iostream>
#include <string>
#include <cstdlib>
#include <list>
#include <map>
#include <math.h>
using namespace std;

#include "worker.h"
#include "logger.h"
#include "WorkerStatistics.h"
class Worker;
class WorkerStatistics;

class Scheduler {
private:
	Logger* log;
	string scheduler_mode;
	float scheduling_interval;
	unsigned short scheduling_interval_for_clock;
	long milliseconds; 
	bool isFirstTime;
	
	map<int,int> taskTimeAverage;
	map<int,int> taskJobCount;
	list<Worker *> workers;    //Contains information of all worker nodes
	list<Job > queuedJobs;    //Contains jobs which the Scheduler has received and have not started running
	list<Job > completedJobs; //Contains jobs which have been completed
	list<Job > runningJobs;   //Contains jobs which are currently in running state
	list<WorkerStatistics *> workerStats; //Contains stats for each worker node
	map< int, list<Job> > spilledJobsMap;
	
	list<Worker *>::iterator j; //iterator used by runScheduler() function
	map<int,int> getJobAwayTime();
	
	
public:
	// constructor
	Scheduler(string scheduler_mode, float scheduling_interval, 
			  unsigned short interval_for_clock);
	
	//this function will start a worker and return its worker_id
	unsigned int startWorkerNode();
	
	//this function will stop a worker and returns 0 on successful execution
	unsigned int stopWorkerNode(unsigned int worker_id);  
	
	// TaskGenerator will submit jobs to the scheduler using this function
	int submitJobs(list<Job > jobs);
	
	// if required, the Simulator can submit new worker nodes to Scheduler 
	int submitWorkers(list<Worker *> workers);
	
	//Runs the scheduler - will be executed at each clock tick by Simulator
	int runScheduler(); 
    
	// a Worker node will notify the Scheduler when a job finishes its execution
	int notifyJobCompletion(unsigned int task_id, unsigned int job_id, int workerid);
	
	// checks if we can terminate the simulation
	bool areAllJobsCompleted();
	
	//Get Current Time
	long getCurrentTime();
	
	//Returns the WorkerStats object for a specific worker. Returns NULL if not found.
	WorkerStatistics* getWorkerStatsForWorker(int workerid);
	
	//runs the Robin web Scheduler
	void roundRobinWeb();

	//runs the Robin web Scheduler
	void runRoundRobinScheduler();

	
	//Do Initializations and stuff - basically whatver stuff that needs to be done before doing actual scheduling
	void doInitAndOtherStuff();
	
	//gathers statistics from all worker nodes
	void gatherStatisticsFromAllWorkers();
	
	//returns true if its time to schedule
	bool isScheduleTime();
	
	//prints the summary of total jobs processed
	void printSummary();
	
	
	//returns the best worker in terms of available memory - you should have guessed
	Worker* getBestWorkerInTermsOfAvailableMemory();
	
	//runs Single Task scheduler
	void runSingleTaskScheduler();
	
	
	
	/*
	 WEB MODE SCHEDULING
	 */
	
	//runs the Web Mode scheduler
	void runWebModeScheduler();
	void runWebModeSchedulerImproved(); 	
	
	long timeTillNextChargingTick(Worker* worker);
	void switchOffIdleWorkers();
	Worker* getWorkerObject(int wid);
	void tryToSendSpilledJobs();
	void markJobsAsStarted(list<Job> jobsForThisWorker, int wid);
	int getMaxWorkerID();
	list<int> startupNewWorkers(int num_nodes);
	list<Job> fetchJobsFromQueue(int num_jobs);
	
	
	long total_job_count;
	long sum_of_job_duration;
	long fastest_job_time;
	long avg_job_duration;
	long slowest_job_time;
	
	//calculate the average job duration - called every time a job completes 
	void calculateAverageJobDuration(long jobduration);
	//calculate the fastest job time - called every time a job completes 
	void calculateFastestJobTime(long jobduration);
	//calculate the slowest job time - called every time a job completes 
	void calculateSlowestJobTime(long jobduration);
	
	//get the  (average, median, fastest job, slowest job)
	long getAverageJobDuration();
	long getFastestJobTime();
	long getSlowestJobTime();
	
	
	/*
	 WEB MODE SCHEUDLING IMPROVED
	 */
	map<int,int> calcJobsToScheduleBasedOnLoad(int num_jobs_to_be_scheduled);
	map<int,double> calcLoadBasedOnNumWorkers();
	void sendQueuedJobsToIdleWorkers();
	int getNumberOfIdleWorkers();
	list<Worker*> getListOfActiveWorkers();
	list<Job> fetchJobsFromQueueRandomly(int num_jobs);
	
	// outputs the current state of a Scheduler object (can be static also; will be decided later on)
	void print();
};


#endif /* ____ */
