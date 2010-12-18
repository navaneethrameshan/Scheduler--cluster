#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <list>
#include <sstream>
#include "Scheduler.h"
#include "WorkerStatistics.h"
using namespace std;

#define CHARGINGTIME 1*60*1000
#define CHARGINGTIME_SINGLE_TASK 1*60*1000

int milliseconds; 
list<Worker *>::iterator j;
bool isFirstTime;

//! Constructor for scheduler
Scheduler::Scheduler(string scheduler_mode, float scheduling_interval, 
                     unsigned short interval_for_clock, int percent_waste)
{
 
	log = Logger::getLogger();
	isFirstTime = true;
	milliseconds = 0;
	this->scheduler_mode = scheduler_mode;
	this->scheduling_interval = scheduling_interval;
	this->scheduling_interval_for_clock = interval_for_clock;
	this->percentage_waste = percent_waste;
	sum_of_job_duration=0;
	total_job_count=0;
	fastest_job_time=1000000;
	avg_job_duration=0;
	slowest_job_time=-1;
	
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
		(*i).setStartedTime();
	    queuedJobs.push_back(*i);	
	}
	return 0;
	
}

//! Used by the Simulator to submit pre-initialized worker nodes to Scheduler in the start of the simulation. Also, it initializes a WorkerStatistics object for each Worker node and adds it to the workerStats list. These statistics are used by the scheduler to make scheduling decisions
int Scheduler::submitWorkers(list<Worker *> workers)
{
    	list<Worker *>::iterator i;
	for(i=workers.begin(); i!=workers.end();++i)
	{
		this->workers.push_back(*i);
		
		/*Creating a WorkerStatistics object and pushing it to the workerStats list*/
		WorkerStatistics *ws = new WorkerStatistics((*i)->getWorkerID(), getCurrentTime());
		workerStats.push_back(ws);
	}
	return 0;
}

long Scheduler::getCurrentTime() 
{
	return milliseconds;
}

list<Worker*> Scheduler::getListOfActiveWorkers() 
{
	
	list<Worker*>::iterator ww;
	list<Worker*> activeWorkers;
	int wcount;
	
	for(ww=workers.begin();ww!=workers.end();ww++)
	{
		if((*ww)->isAcceptingJobs())
		{
			wcount++;
			activeWorkers.push_back((*ww));
		}
	}
	return activeWorkers;
	
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
			list<Worker*> activeWorkers = getListOfActiveWorkers();
			int num_of_workers = activeWorkers.size();
			if (num_of_workers ==0) {
				return;
			}
			int qsize = queuedJobs.size();
			int jobs_per_worker = ((qsize/num_of_workers) == 0 ? 1 : (qsize/num_of_workers));
			
			int count_for_jobs = queuedJobs.size(); 

			//iterating through all workers, sending them jobs as required and accumulating the spilled over jobs
			list<Worker*>::iterator ww;
			for(ww=activeWorkers.begin();ww!=activeWorkers.end();ww++)
			{
				if( count_for_jobs>0 )
				{

					int wid = (*ww)->getWorkerID();
					
					
					//fetching job objects from queue

					list<Job> jobsForThisWorker = fetchJobsFromQueue(jobs_per_worker);
					
					/*sending jobs to this worker*/
					bool successful = (*ww)->submitJobs(jobsForThisWorker);
					if(successful)
					{
						markJobsAsStarted(jobsForThisWorker, wid);
						stringstream s;
						s<<"Scheduler submitted "<<jobsForThisWorker.size()<<
						" jobs to worker "<<wid<<endl;
						log->decision(s.str());
					}
					else
					  {//this would never happen - but handling this case just to be consistent
					    
					  stringstream s;
					  s<<"Unable to submit "<<jobsForThisWorker.size()<<
					    " jobs to worker "<<wid<<endl;
					  log->decision(s.str());		   
					  cout<<"FATAL: jobs should be inserted to queuedJobs"<<endl;
					  exit(1);
					  }
					/*sending jobs code ends here*/
					
				}
				
	
				count_for_jobs--;
				}
				
			}
	}
}


void Scheduler::roundRobinWeb()
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
			list<Job >::iterator i;
	   	    for(i=queuedJobs.begin();i!=queuedJobs.end();i++)
			{
				
				if( 
				   (*j)->isAcceptingJobs())
				{
					//DEBUG - remove
					list<Job> jobs_to_submit;
					jobs_to_submit.push_back(*i);	
					
					
					
					if( (*j)->submitJobs(jobs_to_submit) == true ) //submitting Job to the worker node
					{
						int wid = (*j)->getWorkerID();
						markJobsAsStarted(jobs_to_submit, wid);
						
						queuedJobs.erase(i); //erasing the Job from the queuedJobs
						i--; 
						
						
					}
					
				}
				
				j++; //workers list iterator
				if(j == workers.end()) 
				{
					j = workers.begin();
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
			}
	return totalJobAwayTime;
}


void Scheduler::runSingleTaskScheduler()
{


	
  //sending Spilled Jobs (if any)
  tryToSendSpilledJobs();
	

  //switch off idle workers if required
  switchOffIdleWorkers();
	
  if( (int)queuedJobs.size() == 0 && (int)this->runningJobs.size() == 0 )
    {
      log->decision("Schedeuler is relaxing because there is no work to do, literally.");
      return;
    }

  else 
    {
      if( (int)queuedJobs.size() > 0 && (getSlowestJobTime()==-1) ) 
	{
	  //running the round robin scheduler if no job has completed yet
	   roundRobinWeb();
	}

      
      else if ((int)(queuedJobs.size() > 0) &&  ((int)queuedJobs.size() <= getNumberOfIdleWorkers()) )
	{
	  sendQueuedJobsToIdleWorkers();
	  
	}
      
      else if( (int)queuedJobs.size() > 0 ) 
	{
	  list<Worker *>::iterator ww;
	  int spilled_over_jobs=0;
	  
	  int qsize = queuedJobs.size();
	  int jobs_per_worker;
	  map<int, int> jobsPerWorkerMap = calcJobsToScheduleBasedOnLoad(qsize);
			
	  //when the no. of jobs are less than workers and if all the jobs spill. Then, spilled
	  //jobs get accumulated, although they aren't removed from queue. Prevents this problem.
	  int count_for_jobs = queuedJobs.size(); 
			
	  //iterating through all workers, sending them jobs as required and accumulating the spilled over jobs
	  for(ww=workers.begin();ww!=workers.end();ww++)
	    {
	      int wid = (*ww)->getWorkerID();
			  
	      if((*ww)->isAcceptingJobs() && count_for_jobs>0 && jobsPerWorkerMap[wid] > 0) //new condition added Dec 17th
		{
		  long time_until_next_charging_tick = timeTillNextChargingTick((*ww));
		  jobs_per_worker = jobsPerWorkerMap[wid];
		  long worst_time_required_for_jobs_in_q = jobs_per_worker*getSlowestJobTime();
					

	
					
		  /*checking if we need new nodes or not*/
									     					
		  if(worst_time_required_for_jobs_in_q > time_until_next_charging_tick ) 
		    {
		      //now we have to send some jobs to current worker and some jobs to added to spilled_over_jobs
						
		      int jobs_to_be_sent = ( (time_until_next_charging_tick/getSlowestJobTime()) <= 1 ) ? 1 : (time_until_next_charging_tick/getSlowestJobTime()); 
						

		      //fetching jobs from queue
		      list<Job> jobsForThisWorker = fetchJobsFromQueueRandomly(jobs_to_be_sent);
						
		      /*sending jobs to this worker*/
		      bool successful = (*ww)->submitJobs(jobsForThisWorker);
		      if(successful)
			{
			  markJobsAsStarted(jobsForThisWorker, wid);
			  stringstream s;
			  s<<getCurrentTime()<<"Scheduler submitted "<<jobsForThisWorker.size()<<
			    " jobs to worker "<<wid<<endl;
			  log->decision(s.str());
			}
		      else
			{ //this would never happen - but doing it just to be consistent
			  stringstream s;
			  s<<"Unable to submit "<<jobsForThisWorker.size()<<
			    " jobs to worker "<<wid<<endl;
			  log->decision(s.str());		    
			  cout<<"FATAL: jobs should be inserted to queuedJobs"<<endl;
			  sleep(1);
			  exit(1);
			}
		      /*sending jobs code ends here*/

		      //spilled jobs for each worker are accumulated so that we can decide how many new nods to start for them 
		      spilled_over_jobs += (jobs_per_worker - jobs_to_be_sent);
		    }
					
					
		  else //means that the jobs_per_worker are feasible for this worker
		    {
		      //fetching job objects from queue						
		      list<Job> jobsForThisWorker = fetchJobsFromQueueRandomly(jobs_per_worker);
						
		      /*sending jobs to this worker*/
		      bool successful = (*ww)->submitJobs(jobsForThisWorker);
		      if(successful)
			{
			  markJobsAsStarted(jobsForThisWorker, wid);
			  stringstream s;
			  s<<"Scheduler submitted "<<jobsForThisWorker.size()<<
			    " jobs to existing worker "<<wid<<" because no new workers were required."<<endl;
			  log->decision(s.str());
			}
		      else
			{//this would never happen							
			  stringstream s;
			  s<<"Scheduler was unable to submit "<<jobsForThisWorker.size()<<
			    " jobs to worker "<<wid<<endl;
			  log->decision(s.str());		   
			  cout<<"FATAL: jobs should be inserted to queuedJobs"<<endl;
			  exit(1);
			}
		      /*sending jobs code ends here*/
						
		    }
		}
				
				
	      count_for_jobs -- ;
	    }

	      /*************
	      Handling Spilled Over Jobs 
	      **************/					      	

			
	  if(spilled_over_jobs > 0) 
	    {
	      list<Worker *>::iterator work;
	      list<Worker*> existingWorkers;
	      int count=0;
	      for(work=workers.begin();work!=workers.end();work++)
		{
		  if((!(*work)->isAcceptingJobs()) && ((*work)->getState() != OFFLINE) )
		    {
		      existingWorkers.push_back((*work));
		      count++;
		    }
		}
				
	      if (count>0) { //if there are already nodes that are booting up, spilled_jobs should be sent to them - we dont start new nodes in this case
		int jobsperworker = ((spilled_over_jobs /count ) == 0 ? spilled_over_jobs :spilled_over_jobs/count);
					
		if(spilled_over_jobs < count) {//means that we dont have enough jobs for all workers, so submitting to first available worker
		  Worker *wrkr = existingWorkers.front();
		  list<Job> jobsForThisWorker = fetchJobsFromQueueRandomly(jobsperworker);
		  list<Job> sjobs = spilledJobsMap[wrkr->getWorkerID()];
						
		  list<Job>::iterator it;
		  it = sjobs.begin();
		  sjobs.splice(it, jobsForThisWorker);
		  spilledJobsMap[wrkr->getWorkerID()] = sjobs;

		}
					
		else {//splitting the jobs evenly to all workers which are still booting up
		  list<Worker *>::iterator nw;
		  for(nw=existingWorkers.begin();nw!=existingWorkers.end();nw++)
		    {
		      list<Job> jobsForThisWorker = fetchJobsFromQueueRandomly(jobsperworker);
		      list<Job> sjobs = spilledJobsMap[((*nw)->getWorkerID())];
		      list<Job>::iterator it;
		      it = sjobs.begin();
		      sjobs.splice(it, jobsForThisWorker);
		      spilledJobsMap[(*nw)->getWorkerID()] = sjobs;
		    }
		}
					
	      }
				
				
	      else{ //it means that there are no nodes booting up - so starting up new nodes and send spilled_jobs to them
					
		//calculating the time(in milliseconds) required for spilled over jobs
		long time_for_spilled_jobs = spilled_over_jobs*getSlowestJobTime();
				
		double workers_to_be_started = ceil(((double)time_for_spilled_jobs/(getChargingTimeConsideringPercentWaste()) ));
					

		// all jobs_per_worker (also in all of above) calculations will result in some excess jobs if jobs_per_worker/num_of_workers doesnt divide perfectly
		// however the algorithm will work since the excess jobs will remain in queuedJobs
					
		int jobs_per_new_worker = ((spilled_over_jobs /workers_to_be_started ) == 0 ? spilled_over_jobs :spilled_over_jobs/workers_to_be_started);
					
		//! if jobs per new worker are less than the workers to be started, then startup only jobs_per_new_worker number of nodes	
		workers_to_be_started = (jobs_per_new_worker<workers_to_be_started) ? 1 : workers_to_be_started;
					
		stringstream s;
		s<<"Scheduler will now startup "<<workers_to_be_started<<" new nodes because there are "<<spilled_over_jobs<<" jobs in the queue which the current workers cant finish before the next charging tick";
		log->decision(s.str());
					
		list<int> newWorkerIDs = startupNewWorkers(workers_to_be_started);
		list<int>::iterator n;
					
		//uniformly submitting jobs to the newWorkers
		for(n=newWorkerIDs.begin();n!=newWorkerIDs.end();n++) 
		  {
		    list<Job> jobsForThisWorker = fetchJobsFromQueueRandomly(jobs_per_new_worker);
						
		    //adding it in map. Will be removed when jobs are scheduled
		    spilledJobsMap[ (*n) ] = jobsForThisWorker;
						
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
	print();
	
	//trying to send spilled jobs
	tryToSendSpilledJobs();
	
	//switch off idle workers if required
	switchOffIdleWorkers();
	
	if( (int)queuedJobs.size() == 0 && (int)this->runningJobs.size() == 0 )
    {
		log->decision("Schedeuler is relaxing because there is no work to do, literally.");
		return;
    }
	else 
	  {
	    if( (int)queuedJobs.size() > 0 && (getSlowestJobTime()==-1) ) 
	      {
		//running the round robin scheduler if no job has completed yet
		roundRobinWeb();
	      }
		else if( (int)queuedJobs.size() > 0 ) 
		{
			list<Worker *>::iterator ww;
			int spilled_over_jobs=0;
			int wcount=0;
			
			for(ww=workers.begin();ww!=workers.end();ww++)
			{
				if((*ww)->isAcceptingJobs() /*|| (*ww)->getState() != OFFLINE*/ )
				{
					wcount++;
				}
			}
			int num_of_workers = wcount;
			int qsize = queuedJobs.size();
			long jobs_per_worker = ((qsize/num_of_workers) == 0 ?  1 : (qsize/num_of_workers));
			
			//when the no. of jobs are less than workers and if all the jobs spill. Then, spilled
			//jobs get accumulated, although they aren't removed from queue. Prevents this problem.
			int count_for_jobs = queuedJobs.size(); 
			
			//iterating through all workers, sending them jobs as required and accumulating the spilled over jobs
			for(ww=workers.begin();ww!=workers.end();ww++)
			{
				
				if((*ww)->isAcceptingJobs() && count_for_jobs>0 )
				{
					int wid = (*ww)->getWorkerID();
					long time_until_next_charging_tick = timeTillNextChargingTick((*ww));
					long worst_time_required_for_jobs_in_q = jobs_per_worker*getSlowestJobTime();
					
					
					/*checking if we need new nodes or not*/
										
					
					if(worst_time_required_for_jobs_in_q > time_until_next_charging_tick )
					{
							
						int jobs_to_be_sent = (time_until_next_charging_tick/getSlowestJobTime());
						
						//fetching jobs from queue
						list<Job> jobsForThisWorker = fetchJobsFromQueue(jobs_to_be_sent);
						
						/*sending jobs to this worker*/
						bool successful = (*ww)->submitJobs(jobsForThisWorker);
						if(successful)
						{
							markJobsAsStarted(jobsForThisWorker, wid);
							stringstream s;
							s<<getCurrentTime()<<"Scheduler submitted "<<jobsForThisWorker.size()<<
							" jobs to worker "<<wid<<endl;
							log->decision(s.str());

						}
						else
						  {//this would never happen - but just being consistent
						    stringstream s;
							s<<"Unable to submit "<<jobsForThisWorker.size()<<
							" jobs to worker "<<wid<<endl;
							log->decision(s.str());		    
							cout<<"FATAL: jobs should be inserted to queuedJobs"<<endl;
							exit(1);
						}
						/*sending jobs code ends here*/
						
						spilled_over_jobs += (jobs_per_worker - jobs_to_be_sent);
					}
					
					
					else
					{
						//means that the jobs_per_worker are feasible for this worker

						//fetching job objects from queue						
						list<Job> jobsForThisWorker = fetchJobsFromQueue(jobs_per_worker);
						
						/*sending jobs to this worker*/
						bool successful = (*ww)->submitJobs(jobsForThisWorker);
						if(successful)
						{
							markJobsAsStarted(jobsForThisWorker, wid);
							stringstream s;
							s<<"Submitted "<<jobsForThisWorker.size()<<" jobs to worker "<<wid<<" because these jobs will finish before the next charging tick."<<endl;
							log->decision(s.str());
						}
						else
						{//this would never happen - but just being consistent
							
													stringstream s;
							s<<"Unable to submit "<<jobsForThisWorker.size()<<
							" jobs to worker "<<wid<<endl;
							log->decision(s.str());		   
							cout<<"FATAL: jobs should be inserted to queuedJobs"<<endl;
							exit(1);
						}
						/*sending jobs code ends here*/
						
					}
				}
				
				
				
				count_for_jobs -- ;
			}


			
			/*************
			 Spilled Over Jobs 
			**************/	
			
			
			if(spilled_over_jobs > 0) 
			{
				list<Worker *>::iterator work;
				list<Worker*> existingWorkers;
				int count=0;
				for(work=workers.begin();work!=workers.end();work++)
				{
					if((!(*work)->isAcceptingJobs()) && ((*work)->getState() != OFFLINE) )
					{
						existingWorkers.push_back((*work));
						count++;
					}
				}
				
				if (count>0) { //if there are already nodes that are booting up, spilled_jobs should be sent to them - we dont start new nodes in this case
					int jobsperworker = ((spilled_over_jobs /count ) == 0 ? spilled_over_jobs :spilled_over_jobs/count);
					

					if(spilled_over_jobs < count) {//means that we dont have enough jobs for all workers, so submitting to first available worker
						Worker *wrkr = existingWorkers.front();
						list<Job> jobsForThisWorker = fetchJobsFromQueue(jobsperworker);
						list<Job> sjobs = spilledJobsMap[wrkr->getWorkerID()];						
						list<Job>::iterator it;
						it = sjobs.begin();
						sjobs.splice(it, jobsForThisWorker);
						spilledJobsMap[wrkr->getWorkerID()] = sjobs;
						
					}
					
					else {//splitting the jobs evenly to all workers which are still booting up
						list<Worker *>::iterator nw;
						for(nw=existingWorkers.begin();nw!=existingWorkers.end();nw++)
						{

							list<Job> jobsForThisWorker = fetchJobsFromQueue(jobsperworker);
							list<Job> sjobs = spilledJobsMap[((*nw)->getWorkerID())];
							list<Job>::iterator it;
							it = sjobs.begin();
							sjobs.splice(it, jobsForThisWorker);
							spilledJobsMap[(*nw)->getWorkerID()] = sjobs;
						}
					}
					
				}
				
				
				else{ //it means that there are no nodes booting up - so starting up new nodes and send spilled_jobs to them
					
					
					//calculating the time(in milliseconds) required for spilled over jobs
					long time_for_spilled_jobs = spilled_over_jobs*getSlowestJobTime();
					
					double workers_to_be_started = ceil(((double)time_for_spilled_jobs/(CHARGINGTIME) ));
					
					
					int jobs_per_new_worker = ((spilled_over_jobs /workers_to_be_started ) == 0 ? spilled_over_jobs :spilled_over_jobs/workers_to_be_started);
					
					//! if jobs per new worker are less than the workers to be started, then startup only jobs_per_new_worker number of nodes	
					workers_to_be_started = (jobs_per_new_worker<workers_to_be_started) ? 1 : workers_to_be_started;
					
					stringstream s;
					s<<"Scheduler will now startup "<<workers_to_be_started<<" new nodes because there are "<<spilled_over_jobs<<" jobs which cant be scheduled on existing workers";
					log->decision(s.str());
					
					
					list<int> newWorkerIDs = startupNewWorkers(workers_to_be_started);
					list<int>::iterator n;
					
					//uniformly submitting jobs to the newWorkers
					for(n=newWorkerIDs.begin();n!=newWorkerIDs.end();n++) 
					{
						list<Job> jobsForThisWorker = fetchJobsFromQueue(jobs_per_new_worker);						
						//adding it in map. Will be removed when jobs are scheduled
						spilledJobsMap[ (*n) ] = jobsForThisWorker;
						
					}
				}
				
			}
			
			
		}
    }
}

map<int,double> Scheduler::calcLoadBasedOnNumWorkers()
{
	//fetch number of jobs on each worker node
	list<Worker* >::iterator i;
	map<int,double> workerLoad;
	int total_jobs = 0;

	
	for(i = workers.begin(); i != workers.end(); i++)
    {
		if((*i)->isAcceptingJobs()) 
		{
		int wid = (*i)->getWorkerID();
		WorkerStatistics *ws = getWorkerStatsForWorker(wid);
		int num_sjobs = ws->getNumberOfSubmittedJobs()+1 ;
		total_jobs += num_sjobs;
		
		workerLoad[wid] = num_sjobs;
		
	
		       
		}
			}
		
			map<int,double>::iterator it;
			for(it=workerLoad.begin(); it!= workerLoad.end(); it++)
			{
				workerLoad[(*it).first] = (*it).second/total_jobs;
			}
			
			return workerLoad;
}
			
map<int, int> Scheduler::calcJobsToScheduleBasedOnLoad(int num_jobs_to_be_scheduled)
{
	map<int,double> performanceMap = calcLoadBasedOnNumWorkers();
	map<int,int> jobsToSchedulePerWorker;
	double totalPerformance=0;
	
	
	map<int,double>::iterator it;
	for(it=performanceMap.begin(); it!= performanceMap.end(); it++)
	{
	  performanceMap[(*it).first] = 1/((*it).second);
	  totalPerformance += performanceMap[(*it).first];		
	}
	for(it=performanceMap.begin(); it!= performanceMap.end(); it++)
	{
		jobsToSchedulePerWorker[(*it).first] =  (int) (( num_jobs_to_be_scheduled*performanceMap[(*it).first] ) /totalPerformance);
	}
	
	
	return jobsToSchedulePerWorker;
	
}


void Scheduler::sendQueuedJobsToIdleWorkers()
	    {
	      list<Worker*>::iterator W;
	      W = workers.begin();
	      int i;
	      for(i=0;i<(int)queuedJobs.size();i++)
		{
		  
		  while((*W)->getState() != IDLE && !(*W)->isAcceptingJobs())
		    W++;
		  
		  list<Job> job = fetchJobsFromQueue(1);
		  (*W)->submitJobs(job);
		  markJobsAsStarted(job, (*W)->getWorkerID());
		  
		  W++;
		  if(W == workers.end())
		    W = workers.begin();
		}
	    }



int Scheduler::getNumberOfIdleWorkers()
{
  int idle_count=0;
  list<Worker*>::iterator W;
  for(W=workers.begin();W!=workers.end();W++)
    {
      if((*W)->isAcceptingJobs() && (*W)->getState() == IDLE)
	idle_count++;
    }
  return idle_count;
}

void Scheduler::runWebModeSchedulerImproved(){
	

   //gathering statistics for all workers so that we have a fresh view of all workers
  //  gatherStatisticsFromAllWorkers();
  // print();

	
  // //sending Spilled Jobs (if any)
  tryToSendSpilledJobs();
	

  //switch off idle workers if required
  switchOffIdleWorkers();
	
  if( (int)queuedJobs.size() == 0 && (int)this->runningJobs.size() == 0 )
    {
      log->decision("Schedeuler is relaxing because there is no work to do, literally.");
      return;
    }

  else 
    {
      if( (int)queuedJobs.size() > 0 && (getSlowestJobTime()==-1) ) 
	{
	  //running the round robin scheduler if no job has completed yet
	  stringstream s;
	  s<<"Scheduler will do round robin scheduling until there is at least one completed job"<<endl;
	  log->decision(s.str());
	  roundRobinWeb();
	}

      
      else if ((int)(queuedJobs.size() > 0) &&  ((int)queuedJobs.size() <= getNumberOfIdleWorkers()) )
	{
	  sendQueuedJobsToIdleWorkers();
	  
	}
      
      else if( (int)queuedJobs.size() > 0 ) 
	{

	  list<Worker *>::iterator ww;
	  int spilled_over_jobs=0;
	
		int qsize = queuedJobs.size();
	  int jobs_per_worker;
	  map<int, int> jobsPerWorkerMap = calcJobsToScheduleBasedOnLoad(qsize);
			
	  //when the no. of jobs are less than workers and if all the jobs spill. Then, spilled
	  //jobs get accumulated, although they aren't removed from queue. Prevents this problem.
	  int count_for_jobs = queuedJobs.size(); 
			
	  //iterating through all workers, sending them jobs as required and accumulating the spilled over jobs
	  for(ww=workers.begin();ww!=workers.end();ww++)
	    {
	      int wid = (*ww)->getWorkerID();
			  
	      if((*ww)->isAcceptingJobs() && count_for_jobs>0 && jobsPerWorkerMap[wid] > 0) //new condition added Dec 17th
		{
		  long time_until_next_charging_tick = timeTillNextChargingTick((*ww));
		  jobs_per_worker = jobsPerWorkerMap[wid];
		  long worst_time_required_for_jobs_in_q = jobs_per_worker*getSlowestJobTime();
					
				
		  /*checking if we need new nodes or not*/
		  if(worst_time_required_for_jobs_in_q > time_until_next_charging_tick ) 
		    {
		      //now we have to send some jobs to current worker and some jobs to added to spilled_over_jobs
		      int jobs_to_be_sent = ( (time_until_next_charging_tick/getSlowestJobTime()) <= 1 ) ? 1 : (time_until_next_charging_tick/getSlowestJobTime()); 
						
		      //fetching jobs from queue
		      list<Job> jobsForThisWorker = fetchJobsFromQueueRandomly(jobs_to_be_sent);
						
		      /*sending jobs to this worker*/
		      bool successful = (*ww)->submitJobs(jobsForThisWorker);
		      if(successful)
			{
			  markJobsAsStarted(jobsForThisWorker, wid);
			  stringstream s;
			  s<<getCurrentTime()<<"Scheduler submitted "<<jobsForThisWorker.size()<<
			    " jobs to worker "<<wid<<endl;
			  log->decision(s.str());
			  //jobsForThisWorker.clear();
			}
		      else
			{//this would never happen - just being consistent
			  stringstream s;
			  s<<"Unable to submit "<<jobsForThisWorker.size()<<
			    " jobs to worker "<<wid<<endl;
			  log->decision(s.str());		    
			  cout<<"FATAL: jobs should be inserted to queuedJobs"<<endl;
			  sleep(1);
			  exit(1);
			}
		      /*sending jobs code ends here*/
		      //spilled jobs for each worker are accumulated so that we can decide how many new nods to start for them 
		      spilled_over_jobs += (jobs_per_worker - jobs_to_be_sent);

		    }
					
					
		  else //means that the jobs_per_worker are feasible for this worker
		    {
		      //fetching job objects from queue						
		      list<Job> jobsForThisWorker = fetchJobsFromQueueRandomly(jobs_per_worker);
						
		      /*sending jobs to this worker*/
		      bool successful = (*ww)->submitJobs(jobsForThisWorker);
		      if(successful)
			{
			  markJobsAsStarted(jobsForThisWorker, wid);
			  stringstream s;
			  s<<"Submitted "<<jobsForThisWorker.size()<<
			    " jobs to worker "<<wid<<" because these jobs will complete before the next charging tick"<<endl;
			  log->decision(s.str());
			}
		      else
			{//this would never happen - just being consistent
       			
			  stringstream s;
			  s<<"Unable to submit "<<jobsForThisWorker.size()<<
			    " jobs to worker "<<wid<<endl;
			  log->decision(s.str());		   
			  cout<<"FATAL: jobs should be inserted to queuedJobs"<<endl;
			  exit(1);
			}
		      /*sending jobs code ends here*/
						
		    }
		}
	      count_for_jobs -- ;
	    }

			
	      /*************
	     	Handling Spilled Over Jobs 
	      **************/	
	
			
	  if(spilled_over_jobs > 0) 
	    {
	      list<Worker *>::iterator work;
	      list<Worker*> existingWorkers;
	      int count=0;
	      for(work=workers.begin();work!=workers.end();work++)
		{
		  if((!(*work)->isAcceptingJobs()) && ((*work)->getState() != OFFLINE) )
		    {
		      existingWorkers.push_back((*work));
		      count++;
		    }
		}
				
	      if (count>0) { //if there are already nodes that are booting up, spilled_jobs should be sent to them - we dont start new nodes in this case
		int jobsperworker = ((spilled_over_jobs /count ) == 0 ? spilled_over_jobs :spilled_over_jobs/count);
					
		if(spilled_over_jobs < count) {//means that we dont have enough jobs for all workers, so submitting to first available worker
		  Worker *wrkr = existingWorkers.front();
		  list<Job> jobsForThisWorker = fetchJobsFromQueueRandomly(jobsperworker);
		  list<Job> sjobs = spilledJobsMap[wrkr->getWorkerID()];
						
		  list<Job>::iterator it;
		  it = sjobs.begin();
		  sjobs.splice(it, jobsForThisWorker);
		  spilledJobsMap[wrkr->getWorkerID()] = sjobs;
						
		}
					
		else {//splitting the jobs evenly to all workers which are still booting up
		  list<Worker *>::iterator nw;
		  for(nw=existingWorkers.begin();nw!=existingWorkers.end();nw++)
		    {
		      list<Job> jobsForThisWorker = fetchJobsFromQueueRandomly(jobsperworker);
		      list<Job> sjobs = spilledJobsMap[((*nw)->getWorkerID())];
		      list<Job>::iterator it;
		      it = sjobs.begin();
		      sjobs.splice(it, jobsForThisWorker);
		      spilledJobsMap[(*nw)->getWorkerID()] = sjobs;
		      stringstream s;
		      s<<"Scheduler has now assigned "<<sjobs.size()<<" jobs to worker "<<(*nw)->getWorkerID()<<". Jobs will sent when it has booted up"<<endl;
		      log->decision(s.str());
		    }
		}
					
	      }
				
				
	      else{ //it means that there are no nodes booting up - so starting up new nodes and send spilled_jobs to them
						
		//calculating the time(in milliseconds) required for spilled over jobs
		long time_for_spilled_jobs = spilled_over_jobs*getSlowestJobTime();
		double workers_to_be_started = ceil(((double)time_for_spilled_jobs/(CHARGINGTIME) ));
					

		// all jobs_per_worker (also in all of above) calculations will result in some excess jobs if jobs_per_worker/num_of_workers doesnt divide perfectly
		// however the algorithm will work since the excess jobs will remain in queuedJobs
		int jobs_per_new_worker = ((spilled_over_jobs /workers_to_be_started ) == 0 ? spilled_over_jobs :spilled_over_jobs/workers_to_be_started);
					
		//! if jobs per new worker are less than the workers to be started, then startup only jobs_per_new_worker number of nodes	
		workers_to_be_started = (jobs_per_new_worker<workers_to_be_started) ? 1 : workers_to_be_started;
					
		stringstream s;
		s<<"Scheduler will now startup "<<workers_to_be_started<<" new nodes because there are "<<spilled_over_jobs<<" jobs which wont finish before the next charging tick. So starting new worker(s) here will improve Average response time.";
		log->decision(s.str());
		

		list<int> newWorkerIDs = startupNewWorkers(workers_to_be_started);
		list<int>::iterator n;
					
		//uniformly submitting jobs to the newWorkers
		for(n=newWorkerIDs.begin();n!=newWorkerIDs.end();n++) 
		  {
		    list<Job> jobsForThisWorker = fetchJobsFromQueueRandomly(jobs_per_new_worker);
						
		    //adding it in map. Will be removed when jobs are scheduled
		    spilledJobsMap[ (*n) ] = jobsForThisWorker;
						
		  }
	      }
				
	    }
			
			
	}
    }
	
}

long Scheduler::getChargingTimeConsideringPercentWaste()
{
  return (CHARGINGTIME_SINGLE_TASK)*(1-(double)percentage_waste/100);
}

//returns the time remaaining in the next charging tick for a worker
long Scheduler::timeTillNextChargingTick(Worker* worker)
{
  long current_worker_time = worker->getTotalCPUTime();
  if(scheduler_mode == "W")
    return  ( (CHARGINGTIME) - (current_worker_time%(CHARGINGTIME) ));
  else//(scheduler_mode == "S") {
    return  ( (getChargingTimeConsideringPercentWaste()) - (current_worker_time%(getChargingTimeConsideringPercentWaste()) ));
}


//stops worker nodes which are idle on the 59th minute if there are no queued jobs in the scheduler
void Scheduler::switchOffIdleWorkers()
{
    
    list<Worker*>::iterator it;
    for(it=workers.begin();it!=workers.end();it++)
	{
		if((*it)->isAcceptingJobs())
		{
			int wid = (*it)->getWorkerID();
			long time_since_on = (*it)->getTotalCPUTime();
			if(time_since_on % (59*60*1000) == 0 && queuedJobs.size() == 0)
			{
				//stopping the worker node at 59th Minute if there are no jobs in the schedulers queue
				stopWorkerNode(wid);
			}
		}
	}
	
}

Worker* Scheduler::getWorkerObject(int wid)
{
	list<Worker*>::iterator it;
	
	for(it=workers.begin();it!=workers.end();it++)
    {
		int workerid = (*it)->getWorkerID();
		if(workerid == wid)
			return (*it);
    }
	return NULL;
}

void Scheduler::tryToSendSpilledJobs()
{
	
	if(spilledJobsMap.size() > 0) {

		map<int,list<Job> >::iterator it;
		for(it=spilledJobsMap.begin();it!=spilledJobsMap.end();it++)
		{
			list<Job> jobs = (*it).second;
			int num_jobs = jobs.size();
			int wid = (*it).first;
			Worker *worker = getWorkerObject(wid);
			int wid_copy = wid; 
			
			/*sending jobs to this worker*/
			bool successful = worker->submitJobs(jobs);
			if(successful)
			{
				//marking the jobs as started
				markJobsAsStarted(jobs, wid);
				
				//erasing the entry from the map
				spilledJobsMap.erase(wid);

				stringstream s;
				s<<getCurrentTime()<<"Scheduler has submitted "<<num_jobs<<" jobs to the  worker "<<wid_copy<<" that just started accepting jobs"<<endl;
				log->decision(s.str());
				break;
			}
			else
			{
			  stringstream s;
			  s<<getCurrentTime()<<"Scheduler was unable to submit "<<jobs.size()<<" jobs to worker "<<wid<<" because it is still booting up. Will try again in next scheduling cycle."<<endl;
			  log->decision(s.str());		   
			}
			/*sending jobs code ends here*/
			
		}
		
	}
	
}

void Scheduler::markJobsAsStarted(list<Job> jobsForThisWorker, int wid)
{
	list<Job>::iterator it;
	
	for(it=jobsForThisWorker.begin();it!=jobsForThisWorker.end();it++)
    {
		(*it).setStartTime();
		WorkerStatistics *ws = getWorkerStatsForWorker(wid);
		ws->incrementSubmittedJobs(getCurrentTime());
		ws->incrementTotalSubmittedJobs(getCurrentTime());
		runningJobs.push_back((*it));
    }
}

int Scheduler::getMaxWorkerID()
{
	int max=0;
	list<Worker*>::iterator it;
	for(it=workers.begin();it!=workers.end();it++)
    {
		int wid = (*it)->getWorkerID();
		if( (wid > max) )
		{
			max = wid;
		}
    }
	return max;
}

list<int> Scheduler::startupNewWorkers(int num_nodes)
{
	int i;
	list<int> newWorkerIDs;
	
	for(i=0;i<num_nodes;i++)
    {
		int wid = startWorkerNode();
		newWorkerIDs.push_back(wid);
    }
	return newWorkerIDs;
	
}


//returns num_jobs from the queuedJobs list
list<Job> Scheduler::fetchJobsFromQueue(int num_jobs)
{
	
	list<Job> fetchedJobs;
	int jcount=0;
	
	while(jcount<num_jobs)
	{

		fetchedJobs.push_back(queuedJobs.front());
		queuedJobs.pop_front();
		jcount++;
	}
	
	return fetchedJobs;
}

//fetches jobs randomly from the queue
list<Job> Scheduler::fetchJobsFromQueueRandomly(int num_jobs)
{
	
	list<Job> randJobsList;
	int jcount=0;
	list<Job>::iterator it;
	srand(1);
	
	while(jcount<num_jobs)
	{
		it = queuedJobs.begin();
		int rand_val = rand()%queuedJobs.size();
				
		for(int i=0;i<rand_val;i++)
			it++;
		
		randJobsList.push_back((*it));
		queuedJobs.erase(it);
		jcount++;
	}
	
	return randJobsList;
}

bool Scheduler::isScheduleTime() {

  return (  (milliseconds)%(short)(scheduling_interval*1000) == 0) || (milliseconds == 1   );

}

//! Runs the scheduler (e.g. start Worker nodes, stop Worker nodes, submitJobs) - will be executed at each clock tick by Simulator
int Scheduler::runScheduler()
{ 
	
  //doing initializations and stuff
  doInitAndOtherStuff();
	
  //checking if its time to do some scheduling!
  if(isScheduleTime()) {
	  
    gatherStatisticsFromAllWorkers();
    print();
	  
    if(scheduler_mode == "S")
      {
	runSingleTaskScheduler();
      }
		
    else if(scheduler_mode == "W")
      {
	runWebModeSchedulerImproved();
      }
				
    else if(scheduler_mode == "R")
      {
	runRoundRobinScheduler();
      }		
				
  }
	
  return 0; //returning successful exit everytime (for the time being)
}




WorkerStatistics* Scheduler::getWorkerStatsForWorker(int workerid)
{
	
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
			

			
			map<int,int>::iterator it;
			/*
			 * calculating interesting values for WebModeScheduler
			 */
			
		    
			long jobduration = (*i).getEndTime() - (*i).getStartTime();
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
		if((*i)->isAcceptingJobs()) {
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
		<<"\tsubmittedjobsinprogress:"<<ws->getNumberOfSubmittedJobs()
		 <<"\ttotalsubmittedjobs: "<<ws->getNumberOfTotalSubmittedJobs()
		<<endl;
		}
    }
	
	log->decision(s.str());
	
	
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
