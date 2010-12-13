#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <list>
#include <sstream>
#include "Scheduler.h"
#include "WorkerStatistics.h"
using namespace std;

int milliseconds; 
list<Worker *>::iterator j;
bool isFirstTime;

//! Constructor for scheduler
Scheduler::Scheduler(string scheduler_mode, float scheduling_interval, 
                     unsigned short interval_for_clock)
{
  log = Logger::getLogger();
  isFirstTime = true;
  milliseconds = 0;

  this->scheduler_mode = scheduler_mode;
  this->scheduling_interval = scheduling_interval;
  this->scheduling_interval_for_clock = interval_for_clock;
  sum_of_job_duration=0;
  total_job_count=0;
  fastest_job_time=1000000;
  avg_job_duration=0;
  slowest_job_time=-1;;

  print();

  queuedJobs.clear ();
}
  
//! This function will start a worker and return its worker_id
unsigned int Scheduler::startWorkerNode()
{
  list<Worker *>::iterator i;
  
  for(i=workers.begin(); i!=workers.end();++i  ) {
    if( (*i)->getState() == OFFLINE ) {
      (*i)->startWorker();
      break;
    }
  }
  return (*i)->getWorkerID();
  
}

//! This function will stop a worker and returns 0 on successful execution
  unsigned int Scheduler::stopWorkerNode(unsigned int worker_id)
  {
  list<Worker *>::iterator i;
        
  for(i=workers.begin(); i!=workers.end();++i  ) {
      if( (*i)->getWorkerID() == worker_id ) {
	(*i)->stopWorker();
	break;
      }
    }
    return 0;
  }  
  
  
  //! TaskGenerator will submit jobs to the scheduler using this function
  int Scheduler::submitJobs(list<Job > jobs)
  {
	list<Job >::iterator i;
	for(i=jobs.begin(); i!=jobs.end();++i)
	  {
	    queuedJobs.push_back(*i);	
	  }
	return 0;
      
  }

  //! Used by the Simulator to submit pre-initialized worker nodes to Scheduler in the start of the simulation. Also, it initializes a WorkerStatistics object for each Worker node and adds it to the workerStats list. These statistics are used by the scheduler to make scheduling decisions
  int Scheduler::submitWorkers(list<Worker *> workers)
  {
    //TODO: add a created time and last_updated time to the WorkerStatistics class
       	list<Worker *>::iterator i;
	for(i=workers.begin(); i!=workers.end();++i)
	  {
	this->workers.push_back(*i);

	/*Creating a WorkerStatistics object and pushing it to the workerStats list*/
	/*TODO:(Confirm if this is the correct way of adding objects to lists */
	WorkerStatistics *ws = new WorkerStatistics((*i)->getWorkerID(), getCurrentTime());
	workerStats.push_back(ws);
	  }
	return 0;
  }

long Scheduler::getCurrentTime() 
{
  return milliseconds;
}

void Scheduler::runRoundRobinScheduler()
{
  

    if( (int)queuedJobs.size() == 0 && (int)this->runningJobs.size() == 0 )
      {
	log->decision("Nothing to do, so chilling!");
	return;
      }
    else 
      {
	if( (int)queuedJobs.size() > 0) 
	  {
	    //start nodes for each job in queue
	    list<Job >::iterator i;
	   	    for(i=queuedJobs.begin();i!=queuedJobs.end();i++)
	      {

		if( 
		       (*j)->getState() == COMPUTING 
		       || (*j)->getState() == IDLE 
		       || (*j)->getState() == OFFLINE 
		       || (*j)->getState() == INITIALISING
		       || (*j)->getState() == FINALISING
			)
		      {
			list<Job> jobs_to_submit;
			jobs_to_submit.push_back(*i);	

			

			if( (*j)->submitJobs(jobs_to_submit) == true ) //submitting Job to the worker node
			  {
			    /* TODO: Confirm if this is the correct way of getting an object by reference  and that the changes will be persistent*/
			    WorkerStatistics *ws = getWorkerStatsForWorker((*j)->getWorkerID());
			    ws->incrementSubmittedJobs(getCurrentTime());
			    /**/
			    runningJobs.push_back(*i); //adding the job to runningJobs
                                                     
			    stringstream s;
                            s << "Job ID " << (*i).getJobID() << " submitted to Worker " << (*j)->getWorkerID() << "at time: "<<milliseconds;
                            log->decision(s.str());

			    queuedJobs.erase(i); //erasing the Job from the queuedJobs
			    i--; 
			    j++;
			    if(j == workers.end()) 
			      {
				j = workers.begin();
			      }
			   
			  }
			
		      }

	      }

	  }
      }
 

}

map<int, int> Scheduler::getJobAwayTime() {
	map<int,int> totalJobAwayTime;
	list<Job>::iterator iter;
	for (iter = runningJobs.begin(); iter != runningJobs.end(); ++iter) {
		totalJobAwayTime[(*iter).getTaskID()] = 0;
	}
	for (iter = runningJobs.begin(); iter != runningJobs.end(); ++iter) {
		totalJobAwayTime[(*iter).getTaskID()] += (currentTime - (*iter).getStartTime()); // only one task
		cout << currentTime << " <= Currenttime += Starttime => " << (*iter).getStartedTime() << endl;
	}
	return totalJobAwayTime;
}


void Scheduler::runSingleTaskScheduler()
{
  
    gatherStatisticsFromAllWorkers();
	
	// no job is completed and no jobs in queue
	if ((int)queuedJobs.size() == 0 && (int)completedJobs.size() == 0) {
		// fetch job size ... somehow? 
	}
	
	if ((int)queuedJobs.size() > 0 && (int)completedJobs.size() == 0) {
		// normal round robin
	}
	
	if ((int)completedJobs.size() > 0 && (int)queuedJobs.size() == 0 && (int)runningJobs.size() >0) {
		// Calculate an estimate of time to complete each task
		map<int,int> jobsPerTask;
		map<int,int> estimatedTimeToComplete;
		map<int,int> jobAwayTime;
		list<Job>::iterator iter;
		map<int,int>::iterator it;
		for (iter = runningJobs.begin(); iter != runningJobs.end(); ++iter) {
			jobsPerTask [(*iter).getTaskID()] = 0;
		}
		for (iter = runningJobs.begin(); iter != runningJobs.end(); ++iter) {
			jobsPerTask [(*iter).getTaskID()] = jobsPerTask [(*iter).getTaskID()] + 1;
		} 
		
		jobAwayTime = getJobAwayTime();
		
		for ( it=jobsPerTask.begin() ; it != jobsPerTask.end(); it++ ){
			
			estimatedTimeToComplete[(*it).first] =  (((((*it).second) * (taskTimeAverage[(*it).first]))-(jobAwayTime[(*it).first]) )/ (int)workers.size())>0 ?
													(((((*it).second) * (taskTimeAverage[(*it).first]))-(jobAwayTime[(*it).first]) )/ (int)workers.size()):
													(((((*it).second) * (taskTimeAverage[(*it).first])))/ (int)workers.size()); //fixme math
		
			cout <<"["<<currentTime/1000<<"]"<<"running jobs as per map: ++ " <<((*it).second) << "\n \n";
		} 
 
		
		cout <<"["<<currentTime/1000<<"]"<<"Average Time: ++ " <<(taskTimeAverage[(*it).first]/1000)<<endl;
		cout <<"["<<currentTime/1000<<"]"<<"Job Away Time: ++  " <<jobAwayTime[(*it).first]/1000<< endl;
		cout <<"["<<currentTime/1000<<"]"<<"Time Estimated: ++ " <<(estimatedTimeToComplete[1]/1000)+ currentTime/1000<< endl;
		cout <<"["<<currentTime/1000<<"]"<<"running jobs size: ++ " <<runningJobs.size() << "\n \n";
//		cout <<"["<<currentTime/1000<<"]"<<"running jobs as per map: ++ " <<((*it).second) << "\n \n";	

		/*if (estimatedTimeToComplete > 3600*1000) {
			// start nodes 
			int numberOfNodesToStart = ((int)runningJobs.size() *(taskTimeAverage[1]-getJobAwayTime())) / 3600*1000;
			// shift jobs
			cout << "startnodes ++ " << numberOfNodesToStart << "t avg" << taskTimeAverage[1] << endl;
		}*/
	}
	
	
/*	if ((int)completedJobs.size() > 0 && (int)queuedJobs.size() > 0 && (int)runningJobs.size() >0 ) {
		int estimatedTimeToComplete =  (((((int)runningJobs.size()+(int)queuedJobs.size()) * (taskTimeAverage[1]))-getJobAwayTime() )/ (int)workers.size())>0 ?
									   (((((int)runningJobs.size()+(int)queuedJobs.size()) * (taskTimeAverage[1]))-getJobAwayTime() )/ (int)workers.size()):
									   (((((int)runningJobs.size()+(int)queuedJobs.size()) * (taskTimeAverage[1])))/ (int)workers.size()); //fixme math		
		
		if (estimatedTimeToComplete > 3600*1000) {
			// start nodes
			int numberOfNodesToStart = (((int)runningJobs.size() + (int)queuedJobs.size()) * taskTimeAverage[1]) / 3600*1000;
		//	cout << "startnodes -" << numberOfNodesToStart << endl;
			
		}
		
	} */
    
    if( (int)queuedJobs.size() == 0 && (int)this->runningJobs.size() == 0 )
      {
	log->decision("Nothing to do, so chilling!");
	return;
      }
    else 
      {
	if( (int)queuedJobs.size() > 0) 
	  {
	    //start nodes for each job in queue
	    list<Job >::iterator i;
	   	    for(i=queuedJobs.begin();i!=queuedJobs.end();i++)
	      {

		if( (*j)->isAcceptingJobs())
		      {
			list<Job> jobs_to_submit;
			jobs_to_submit.push_back(*i);	

			

			if( (*j)->submitJobs(jobs_to_submit) == true ) //submitting Job to the worker node
			  {
				  // set start time of job
				  (*i).setStartTime();
				  
			    /* TODO: Confirm if this is the correct way of getting an object by reference  and that the changes will be persistent*/
			    WorkerStatistics *ws = getWorkerStatsForWorker((*j)->getWorkerID());
			    ws->incrementSubmittedJobs(getCurrentTime());
			    /**/
			    runningJobs.push_back(*i); //adding the job to runningJobs
                                                     
			    stringstream s;
                            s << "Job ID " << (*i).getJobID() << " submitted to Worker " << (*j)->getWorkerID() << "at time: "<<milliseconds;
                            log->decision(s.str());

			    queuedJobs.erase(i); //erasing the Job from the queuedJobs
			    i--; 
			    j++;
			    if(j == workers.end()) 
			      {
				j = workers.begin();
			      }
			   
			  }
			
		      }

	      }

	  }
      }
 

}

//! Do Initializations and stuff - basically whatver stuff that needs to be done before doing actual scheduling
void Scheduler::doInitAndOtherStuff()
{
  //incrementing time
  milliseconds++;
  
  if((milliseconds)%scheduling_interval_for_clock == 0 || milliseconds == 1) 
    {
      
      if(isFirstTime == true ) {
	j = workers.begin();
	isFirstTime = false;
      }

    }
}

Worker* Scheduler::getBestWorkerInTermsOfAvailableMemory() {

  Worker *bestWorker;
  list<Worker *>::iterator w;
  int bestMemory = -1;
  
  for(w=workers.begin();w!=workers.end();w++) {
    //int wid = (*w)->getWorkerID();
    if(/*(*w)->isAcceptingJobs()*/true)
      {
    int availmem = (*w)->getAvailableMemory();
    if(availmem > bestMemory)
    bestMemory = availmem;
    bestWorker = (*w);
      }
      }
  if(bestMemory > -1)
    {
      return bestWorker;
    }
  else 
    {
      return NULL;
    }
}

void Scheduler::runWebModeScheduler() 
{
  //gathering statistics for all workers so that we have a fresh view of all workers
  gatherStatisticsFromAllWorkers();
  cout<<"["<<getCurrentTime()<<"[AVGRESPTIME]"<<getAverageJobDuration()/1000<<endl;

  if( (int)queuedJobs.size() == 0 && (int)this->runningJobs.size() == 0 )
    {
      log->decision("Schedeuler is relaxing because there is no work to do, literally.");
      return;
    }
  else 
    {
      if( (int)queuedJobs.size() > 0) {
	//getting the size of queuedJobs
	int qsize = queuedJobs.size();

	while(qsize > 0)
	  {
	    //getting the best worker
	    Worker *bestWorker = getBestWorkerInTermsOfAvailableMemory();

	    //getting a Job from the queue
	    Job job = queuedJobs.front();
	    queuedJobs.pop_front();
	    
	    //submitting the job to the worker
	    list<Job> jobs_to_submit;
	    jobs_to_submit.push_back(job);
	    bool successfully_submitted =  bestWorker->submitJobs(jobs_to_submit);
	    if (successfully_submitted) 
	      {
		job.setStartTime();
		WorkerStatistics *ws = getWorkerStatsForWorker(bestWorker->getWorkerID());
		ws->incrementSubmittedJobs(getCurrentTime());
		runningJobs.push_back(job);
	      }
	    else {//we werent able to submit job to worker so putting it back to the queuedJobs list
	      queuedJobs.push_back(job);
	      stringstream s;
	      s<<"Couldn't submit job_id "<<job.getJobID()<<
		" to the worker "<<bestWorker->getWorkerID();
		log->decision(s.str());
	    }
	    qsize--;
	  } 
      }
    }
}

bool Scheduler::isScheduleTime() {
  return (  ((milliseconds)%scheduling_interval_for_clock == 0) || (milliseconds == 1)   );
}

  //! Runs the scheduler (e.g. start Worker nodes, stop Worker nodes, submitJobs) - will be executed at each clock tick by Simulator
int Scheduler::runScheduler()
{ 
  
  //doing initializations and stuff
  doInitAndOtherStuff();

  //checking if its time to do some scheduling!
  if(isScheduleTime()) {
    

    if(scheduler_mode == "S")
      {
	runSingleTaskScheduler();
      }
    
    else if(scheduler_mode == "W")
      {
	//running the web mode scheduler
	runWebModeScheduler();

      }
    
    else if(scheduler_mode == "R")
   {
	//running the round robin scheduler
	runRoundRobinScheduler();
   }
    //gathering statistics of all workers
    gatherStatisticsFromAllWorkers();
    
  }
  
  return 0; //returning successful exit everytime (for the time being)
}


    

WorkerStatistics* Scheduler::getWorkerStatsForWorker(int workerid)
{

  //TODO: confirm if this is the correct way for returning an object by reference
  list<WorkerStatistics *>::iterator w;
  for(w=workerStats.begin();w!=workerStats.end();w++ )
    {
      if ( (*w)->getWorkerID() == workerid)
	{
	  return *w;
	}
    }
  return NULL;
}
			

//! A Worker node will notify the Scheduler when a job finishes its execution
int Scheduler::notifyJobCompletion(unsigned int task_id, unsigned int job_id, int workerid)
{

  //find the job_id in the runningJobs List
  list<Job >::iterator i;
  for(i=runningJobs.begin();i!=runningJobs.end();++i)
      {
	if( (*i).getTaskID() == task_id && (*i).getJobID() == job_id )
	  {
		  (*i).setEndTime();
		  
		  taskJobCount[(*i).getTaskID()] += 1;
		  taskTimeAverage[(*i).getTaskID()] = (taskTimeAverage[(*i).getTaskID()] + ((*i).getEndTime() - (*i).getStartTime())) /
												taskJobCount[(*i).getTaskID()];
		  
		  //		  cout << (*i).getEndTime()/1000 << " End +-- Start " << (*i).getStartTime()/1000 << endl;
		  
		  map<int,int>::iterator it;
		  cout << "foo contains:\n";
		  for ( it=taskTimeAverage.begin() ; it != taskTimeAverage.end(); it++ )
		    // cout << (*it).first << " TaskId +--Avg. Time " << (*it).second/1000 << endl;
		  

	    /*
	     * calculating interesting values for WebModeScheduler
	     */

	    long jobduration = (*i).getEndTime() - (*i).getStartTime();
		  cout<<jobduration<<endl;
	    calculateAverageJobDuration(jobduration);
	    calculateFastestJobTime(jobduration);
	    calculateSlowestJobTime(jobduration);

	    /*Finished calculating values for WebMode Scheduler*/
		  
	    completedJobs.push_back(*i); //marking the job as complete
	    runningJobs.erase(i); //erasing the job from runningJobs 
	    --i; 
	    

	    WorkerStatistics *ws = getWorkerStatsForWorker(workerid);
	    ws->decrementSubmittedJobs(getCurrentTime());

	  }
      }
    return 0; //returning 0 always (for the time being)

  }

void Scheduler::gatherStatisticsFromAllWorkers() {

  long currenttime = getCurrentTime();
  stringstream s;
 
  list<Worker* >::iterator i;
  for(i = workers.begin(); i != workers.end(); i++)
    {
      int wid = (*i)->getWorkerID();
      int qjobs = (*i)->getQueuedJobs();
      double avgresptime = (*i)->getAverageResponseTime();
      float cost = (*i)->getTotalCost();
      int availmem = (*i)->getAvailableMemory();
      
      WorkerStatistics *ws = getWorkerStatsForWorker(wid);
      ws->setNumberOfQueuedJobs(qjobs, currenttime);
      ws->setAverageResponseTime(avgresptime, currenttime);
      ws->setWorkerCostTillNow(cost, currenttime);
      ws->setAvailableMemory(availmem, currenttime);
      
      s<<"[WORKERSTATS]\t(time:"<<getCurrentTime()<<")"
       <<"\twid:"<<wid
       <<"\tqjobs:"<<ws->getNumberOfQueuedJobs()
	<<"\tavgresptime:"<<ws->getAverageResponseTime()
	<<"\tcost:"<<ws->getWorkerCostTillNow()
	<<"\tavailmem:"<<ws->getAvailableMemory()
       <<"\tsubmittedjobs:"<<ws->getNumberOfSubmittedJobs()
	<<endl;
    }

  //    if(getCurrentTime()%100 == 0)
  //  {
  //DEBUG statement
  cout<<s.str()<<endl; //TODO: should be sent to logger instead
  //  }
  
}

bool Scheduler::areAllJobsCompleted() {
  return ((queuedJobs.size() + runningJobs.size()) == 0);
}



  /*
    WEB MODE SCHEDULING - methods
   */

void Scheduler::calculateAverageJobDuration(long jobduration) 
{
  total_job_count++;
  sum_of_job_duration += jobduration;
  //avg_job_duration = (avg_job_duration+jobduration)/2;
  avg_job_duration = sum_of_job_duration/total_job_count;
}
  void Scheduler::calculateFastestJobTime(long jobduration)
  {
    if(jobduration < fastest_job_time)
      fastest_job_time = jobduration;
  }
  void Scheduler::calculateSlowestJobTime(long jobduration)
  {
    if(jobduration > slowest_job_time)
      slowest_job_time = jobduration;
  }
  
  long Scheduler::getAverageJobDuration()
  {
    return ceil(avg_job_duration);
  }
long Scheduler::getFastestJobTime()
  {
    //returns 1000000 if no job has completed yet
    return fastest_job_time;
  }
long Scheduler::getSlowestJobTime()
  {
    //returns -1 if no job has completed yet
    return slowest_job_time;
  }



  //! Outputs the current state of a Scheduler object (can be static also; will be decided later on)
void Scheduler::print()
{
  
  
log->status(scheduler_mode, scheduling_interval, 
              (int)queuedJobs.size(),
              (int)runningJobs.size(),
              (int)completedJobs.size());

  /*  if(workerStats.front() != NULL)
    workerStats.front()->print();
  if(workerStats.back() != NULL)
    workerStats.back()->print();
  */
  
}

void Scheduler::printSummary()
{
  stringstream s;
  s<<" [SUMMARY] (time:"<<getCurrentTime()<<")"
   <<"\tTotal Jobs Completed: "<<completedJobs.size()
   <<""
   <<endl;
  log->decision(s.str());
}
