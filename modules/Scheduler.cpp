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
    
    //running the roundrobin scheduler
    runRoundRobinScheduler();
    
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
int Scheduler::notifyJobCompletion(unsigned int job_id, int workerid)
{
    


  //find the job_id in the runningJobs List
  list<Job >::iterator i;
  for(i=runningJobs.begin();i!=runningJobs.end();++i)
      {
	if( (*i).getJobID() == job_id )
	  {
	    completedJobs.push_back(*i); //marking the job as complete
	    runningJobs.erase(i); //erasing the job from runningJobs 
	    --i; //Added during testing (Navaneeth and Marcus)	

	    /* TODO: Confirm if this is the correct way of getting an object by reference  and that the changes will be persistent*/
	    //fetching the WorkerStatistics for workerid and decrementing submitted jobs
	    WorkerStatistics *ws = getWorkerStatsForWorker(workerid/*TODO: waiting for change from Marcus*/);
	    ws->decrementSubmittedJobs(getCurrentTime());
	    /**/
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
	<<endl;
      
    }

  //DEBUG statement
  cout<<s.str()<<endl; //TODO: should be sent to logger instead
  
}

bool Scheduler::areAllJobsCompleted() {
  return ((queuedJobs.size() + runningJobs.size()) == 0);
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
