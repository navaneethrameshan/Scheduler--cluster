#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <list>
#include <sstream>
#include "Scheduler.h"
#include "WorkerStatistics.h"
using namespace std;

#define CHARGINGTIME 20*60*1000

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
			
			//TODO: Webmode see if this is required or not
			/*
			 WorkerStatistics *ws = new WorkerStatistics((*i)->getWorkerID(), getCurrentTime());
			 workerStats.push_back(ws);
			 */
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
				   /*(*j)->getState() == COMPUTING 
					|| (*j)->getState() == IDLE 
					//|| (*j)->getState() == OFFLINE 
					|| (*j)->getState() == INITIALISING
					|| (*j)->getState() == FINALISING
					*/
				   (*j)->isAcceptingJobs())
				{
					//DEBUG - remove
					cout<<"[SCHEDULER] Worker ID "<<(*j)->getWorkerID()<<" is accepting jobs\n";
					list<Job> jobs_to_submit;
					jobs_to_submit.push_back(*i);	
					
					
					
					if( (*j)->submitJobs(jobs_to_submit) == true ) //submitting Job to the worker node
					{
						int wid = (*j)->getWorkerID();
						markJobsAsStarted(jobs_to_submit, wid);
						stringstream s;
						s << "Job ID " << (*i).getJobID() << " submitted to Worker " << (*j)->getWorkerID() << "at time: "<<milliseconds;
						log->decision(s.str());
						
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
	
	//TODO: time calculation should also include the number of jobs running on the worker node
	
	//TODO: declare all funcs used here in Scheduler.h
	
	//gathering statistics for all workers so that we have a fresh view of all workers
	//    gatherStatisticsFromAllWorkers();
	
	
	//cout<<"["<<getCurrentTime()<<"][AVGRESPTIME] "<<getAverageJobDuration()/1000<<endl;
	
	//sending Spilled Jobs (if any)
	
	tryToSendSpilledJobs();
	
	// cout<<"Webmode crashed? after function call?\n";
	
	//switch off idle workers if required
	//	switchOffIdleWorkers();
	
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
			runRoundRobinScheduler();
			cout<<getCurrentTime()<<" Webmode round roubin scheduler executed\n";
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
					cout<<" Count remaining: "<< count_for_jobs <<endl;
					int wid = (*ww)->getWorkerID();
					long time_until_next_charging_tick = timeTillNextChargingTick((*ww));
					long worst_time_required_for_jobs_in_q = jobs_per_worker*getSlowestJobTime();
					
					//cout<<getCurrentTime()<<" Webmode slowest job time: "<<getSlowestJobTime()<<endl;
					cout<<"Webmode jobs_per_worker "<<jobs_per_worker<<endl;
					cout<<"Webmode worst_time_for_jobs: "<<worst_time_required_for_jobs_in_q<<endl;
					cout<<"Webmode time_until_next_tick: "<<time_until_next_charging_tick<<endl;
					
					/*checking if we need new nodes or not*/
					
					
					cout<<getCurrentTime()<<" queuedJobs.size() "<<queuedJobs.size()<<endl;
					
					//TODO: problem: when the no. of jobs are less than workers and if all the jobs spill. Then, spilled
					//jobs get accumulated, although they aren't removed from queue. CAN CAUSE MALLOC ERROR!! Temporarily,
					// this problem is handled with the variable count_for_jobs.
					
					if(worst_time_required_for_jobs_in_q > time_until_next_charging_tick )
					{
						cout<<"Webmode should spill"<<endl;
						//now we have to send some jobs to current worker and some jobs to added to spilled_over_jobs
						//int jobs_to_be_sent = (time_until_next_charging_tick/worst_time_required_for_jobs_in_q);
						
						int jobs_to_be_sent = (time_until_next_charging_tick/getSlowestJobTime());
						
						cout<<"Webmode Jobs to be sent: "<<jobs_to_be_sent<<endl;
						
						cout<<getCurrentTime()<<" queuedJobs.size() 2:"<<queuedJobs.size()<<endl;
						//fetching jobs from queue
						list<Job> jobsForThisWorker = fetchJobsFromQueue(jobs_to_be_sent);
						
						/*sending jobs to this worker*/
						bool successful = (*ww)->submitJobs(jobsForThisWorker);
						if(successful)
						{
							markJobsAsStarted(jobsForThisWorker, wid);
							stringstream s;
							s<<getCurrentTime()<<" Submitted "<<jobsForThisWorker.size()<<
							" jobs to worker "<<wid<<endl;
							log->decision(s.str());
							//jobsForThisWorker.clear();
						}
						else
						{
							//TODO: insert jobs back to queuedJobs ******************** 
							stringstream s;
							s<<"Unable to submit "<<jobsForThisWorker.size()<<
							" jobs to worker "<<wid<<endl;
							log->decision(s.str());		    
							cout<<"FATAL: jobs should be inserted to queuedJobs"<<endl;
							exit(1);
						}
						/*sending jobs code ends here*/
						
						spilled_over_jobs += (jobs_per_worker - jobs_to_be_sent);
						cout<<getCurrentTime()<<" Webmode spilled_over_jobs accumulated: "<<
						spilled_over_jobs<<endl;
					}
					
					
					else
					{
						//means that the jobs_per_worker are feasible for this worker
						cout<<"jobs_per_worker "<<jobs_per_worker<<endl;
						cout<<getCurrentTime()<<" queuedJobs.size() 3: "<<queuedJobs.size()<<endl;
						//fetching job objects from queue
						
						list<Job> jobsForThisWorker = fetchJobsFromQueue(jobs_per_worker);
						
						/*sending jobs to this worker*/
						bool successful = (*ww)->submitJobs(jobsForThisWorker);
						if(successful)
						{
							markJobsAsStarted(jobsForThisWorker, wid);
							stringstream s;
							s<<"Submitted "<<jobsForThisWorker.size()<<
							" jobs to worker "<<wid<<endl;
							log->decision(s.str());
						}
						else
						{
							
							//TODO: insert jobs back to queue *************** 
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
				
				
				/*************
				 Spilled Over Jobs 
				 **************/	
				
				/*
				 * Startup new workers depending on the number of spill_over_jobs
				 * TODO(DONE): implement list<int> startupNewNodes(int num_nodes);
				 *             -should return an array of worker objects
				 *TODO(DONE): implement list<Job> fetchJobsFromQueue(int num_jobs);
				 *TODO:(DONE) implement a new map for storing spilled over jobs
				 *            -map<int, list<Job>>
				 *            -the scheduler will try to submit the list to the respective
				 *             worker on every scheduling interval. This would ensure that
				 *             the jobs are submitted to the new workers as soon as they
				 *             are up.  
				 *
				 */
				
				count_for_jobs -- ;
			}
			
			if(spilled_over_jobs > 0) //TODO: need to fine tune this value 
			{
				cout << "Spilled over jobs: "<<spilled_over_jobs<<" Queued Jobs size: "<<queuedJobs.size()<<endl ;
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
					
					//TODO: improvement: If there are 4 jobs and 10 workers, all 4 jobs sent to first worker. Instead send 4 jobs to 4 workers, 1 each.
					if(spilled_over_jobs < count) {//means that we dont have enough jobs for all workers, so submitting to first available worker
						Worker *wrkr = existingWorkers.front();
						cout<<getCurrentTime()<<" queuedJobs.size() 4:"<<queuedJobs.size()<<endl;
						list<Job> jobsForThisWorker = fetchJobsFromQueue(jobsperworker);
						list<Job> sjobs = spilledJobsMap[wrkr->getWorkerID()];
						
						cout<<"Webmode if spill sjobs size before splicing: "<<sjobs.size()<<endl;
						list<Job>::iterator it;
						it = sjobs.begin();
						sjobs.splice(it, jobsForThisWorker);
						spilledJobsMap[wrkr->getWorkerID()] = sjobs;
						cout<<"Webmode spill sjobs size after splicing: "<<sjobs.size()<<endl;
						cout<<"Webmode map after splicing: "<<spilledJobsMap[wrkr->getWorkerID()].size()<<endl;
						
					}
					
					else {//splitting the jobs evenly to all workers which are still booting up
						list<Worker *>::iterator nw;
						for(nw=existingWorkers.begin();nw!=existingWorkers.end();nw++)
						{
							cout<<getCurrentTime()<<" queuedJobs.size() 5:"<<queuedJobs.size()<<endl;
							list<Job> jobsForThisWorker = fetchJobsFromQueue(jobsperworker);
							list<Job> sjobs = spilledJobsMap[((*nw)->getWorkerID())];
							cout<<"Webmode else spill sjobs size before splicing: "<<sjobs.size()<<endl;
							list<Job>::iterator it;
							it = sjobs.begin();
							sjobs.splice(it, jobsForThisWorker);
							spilledJobsMap[(*nw)->getWorkerID()] = sjobs;
							cout<<"Webmode spill sjobs size after splicing: "<<sjobs.size()<<endl;
							cout<<"Webmode map after splicing: "<<spilledJobsMap[(*nw)->getWorkerID()].size()<<endl;
						}
					}
					
				}
				
				
				else{ //it means that there are no nodes booting up - so starting up new nodes and send spilled_jobs to them
					cout<<"Webmode Get slowest job in milliseconds: "<<
					getSlowestJobTime()<< endl;
					
					cout<<"Webmode spilled_jobs "<<
					spilled_over_jobs<< endl;
					
					//calculating the time(in milliseconds) required for spilled over jobs
					long time_for_spilled_jobs = spilled_over_jobs*getSlowestJobTime();
					cout<<"Webmode time_for_spilled_jobs (in milliseconds): "<<
					time_for_spilled_jobs<<
					endl;
					
					double workers_to_be_started = ceil(((double)time_for_spilled_jobs/(CHARGINGTIME) ));
					cout<<"Webmode workers_to_be_started "<<
					workers_to_be_started<<
					endl;
					
					//TODO: for improvement
					// all jobs_per_worker (also in all of above) calculations will result in some excess jobs if jobs_per_worker/num_of_workers doesnt divide perfectly
					// however the algorithm will work since the excess jobs will remain in queuedJobs
					
					int jobs_per_new_worker = ((spilled_over_jobs /workers_to_be_started ) == 0 ? spilled_over_jobs :spilled_over_jobs/workers_to_be_started);
					
					//! if jobs per new worker are less than the workers to be started, then startup only jobs_per_new_worker number of nodes	
					//TODO: Problem: The value 1 is a hack to get this working. Must be improved!
					workers_to_be_started = (jobs_per_new_worker<workers_to_be_started) ? 1 : workers_to_be_started;
					
					stringstream s;
					s<<"Webmode Scheduler will now startup "<<workers_to_be_started<<" new nodes because there are "<<spilled_over_jobs<<" spilled over jobs";
					log->decision(s.str());
					
					//TODO: problem: Handle case where, if the no. of workers to be started is more than the no. of workers in the config file.
					//causes SEG FAULT, if not handled!
					
					list<int> newWorkerIDs = startupNewWorkers(workers_to_be_started);
					cout<<getCurrentTime()<<" Webmode Started "<<newWorkerIDs.size()<<" workers\n";
					cout << "Jobs per new worker is: "<<jobs_per_new_worker<<endl;
					list<int>::iterator n;
					
					//uniformly submitting jobs to the newWorkers
					for(n=newWorkerIDs.begin();n!=newWorkerIDs.end();n++) 
					{
						cout<<getCurrentTime()<<" Webmode new worker created\n";
						cout<<getCurrentTime()<<" queuedJobs.size() 6:"<<queuedJobs.size()<<endl;
						list<Job> jobsForThisWorker = fetchJobsFromQueue(jobs_per_new_worker);
						
						//adding it in map. Will be removed when jobs are scheduled
						cout<<getCurrentTime()<<" Webmode Added list of spilled jobs to worker ID: "<< (*n) <<endl;
						spilledJobsMap[ (*n) ] = jobsForThisWorker;
						
					}
				}
				
			}
			
			
		}
    }
}

void Scheduler::runWebModeSchedulerImproved() 
{
	
	//TODO: time calculation should also include the number of jobs running on the worker node
	
	//TODO: declare all funcs used here in Scheduler.h
	
	
	tryToSendSpilledJobs();
	
	// cout<<"Webmode crashed? after function call?\n";
	
	//switch off idle workers if required
	//	switchOffIdleWorkers();
	
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
			runRoundRobinScheduler();
			cout<<getCurrentTime()<<" Webmode round roubin scheduler executed\n";
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
					cout<<" Count remaining: "<< count_for_jobs <<endl;
					int wid = (*ww)->getWorkerID();
					long time_until_next_charging_tick = timeTillNextChargingTick((*ww));
					long worst_time_required_for_jobs_in_q = jobs_per_worker*getSlowestJobTime();
					
					//cout<<getCurrentTime()<<" Webmode slowest job time: "<<getSlowestJobTime()<<endl;
					cout<<"Webmode jobs_per_worker "<<jobs_per_worker<<endl;
					cout<<"Webmode worst_time_for_jobs: "<<worst_time_required_for_jobs_in_q<<endl;
					cout<<"Webmode time_until_next_tick: "<<time_until_next_charging_tick<<endl;
					
					/*checking if we need new nodes or not*/
					
					
					cout<<getCurrentTime()<<" queuedJobs.size() "<<queuedJobs.size()<<endl;
					
					//TODO: problem: when the no. of jobs are less than workers and if all the jobs spill. Then, spilled
					//jobs get accumulated, although they aren't removed from queue. CAN CAUSE MALLOC ERROR!! Temporarily,
					// this problem is handled with the variable count_for_jobs.
					
					if(worst_time_required_for_jobs_in_q > time_until_next_charging_tick )
					{
						cout<<"Webmode should spill"<<endl;
						//now we have to send some jobs to current worker and some jobs to added to spilled_over_jobs
						//int jobs_to_be_sent = (time_until_next_charging_tick/worst_time_required_for_jobs_in_q);
						
						int jobs_to_be_sent = (time_until_next_charging_tick/getSlowestJobTime());
						
						cout<<"Webmode Jobs to be sent: "<<jobs_to_be_sent<<endl;
						
						cout<<getCurrentTime()<<" queuedJobs.size() 2:"<<queuedJobs.size()<<endl;
						//fetching jobs from queue
						list<Job> jobsForThisWorker = fetchJobsFromQueue(jobs_to_be_sent);
						
						/*sending jobs to this worker*/
						bool successful = (*ww)->submitJobs(jobsForThisWorker);
						if(successful)
						{
							markJobsAsStarted(jobsForThisWorker, wid);
							stringstream s;
							s<<getCurrentTime()<<" Submitted "<<jobsForThisWorker.size()<<
							" jobs to worker "<<wid<<endl;
							log->decision(s.str());
							//jobsForThisWorker.clear();
						}
						else
						{
							//TODO: insert jobs back to queuedJobs ******************** 
							stringstream s;
							s<<"Unable to submit "<<jobsForThisWorker.size()<<
							" jobs to worker "<<wid<<endl;
							log->decision(s.str());		    
							cout<<"FATAL: jobs should be inserted to queuedJobs"<<endl;
							exit(1);
						}
						/*sending jobs code ends here*/
						
						spilled_over_jobs += (jobs_per_worker - jobs_to_be_sent);
						cout<<getCurrentTime()<<" Webmode spilled_over_jobs accumulated: "<<
						spilled_over_jobs<<endl;
					}
					
					
					else
					{
						//means that the jobs_per_worker are feasible for this worker
						cout<<"jobs_per_worker "<<jobs_per_worker<<endl;
						cout<<getCurrentTime()<<" queuedJobs.size() 3: "<<queuedJobs.size()<<endl;
						//fetching job objects from queue
						
						list<Job> jobsForThisWorker = fetchJobsFromQueue(jobs_per_worker);
						
						/*sending jobs to this worker*/
						bool successful = (*ww)->submitJobs(jobsForThisWorker);
						if(successful)
						{
							markJobsAsStarted(jobsForThisWorker, wid);
							stringstream s;
							s<<"Submitted "<<jobsForThisWorker.size()<<
							" jobs to worker "<<wid<<endl;
							log->decision(s.str());
						}
						else
						{
							
							//TODO: insert jobs back to queue *************** 
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
			
			if(spilled_over_jobs > 0) //TODO: need to fine tune this value 
			{
				cout << "Spilled over jobs: "<<spilled_over_jobs<<" Queued Jobs size: "<<queuedJobs.size()<<endl ;
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
					
					//TODO: improvement: If there are 4 jobs and 10 workers, all 4 jobs sent to first worker. Instead send 4 jobs to 4 workers, 1 each.
					if(spilled_over_jobs < count) {//means that we dont have enough jobs for all workers, so submitting to first available worker
						Worker *wrkr = existingWorkers.front();
						cout<<getCurrentTime()<<" queuedJobs.size() 4:"<<queuedJobs.size()<<endl;
						list<Job> jobsForThisWorker = fetchJobsFromQueue(jobsperworker);
						list<Job> sjobs = spilledJobsMap[wrkr->getWorkerID()];
						
						cout<<"Webmode if spill sjobs size before splicing: "<<sjobs.size()<<endl;
						list<Job>::iterator it;
						it = sjobs.begin();
						sjobs.splice(it, jobsForThisWorker);
						cout<<"Webmode if spill sjobs size before splicing: "<<sjobs.size()<<endl;
						
					}
					
					else {//splitting the jobs evenly to all workers which are still booting up
						list<Worker *>::iterator nw;
						for(nw=existingWorkers.begin();nw!=existingWorkers.end();nw++)
						{
							cout<<getCurrentTime()<<" queuedJobs.size() 5:"<<queuedJobs.size()<<endl;
							list<Job> jobsForThisWorker = fetchJobsFromQueue(jobsperworker);
							list<Job> sjobs = spilledJobsMap[((*nw)->getWorkerID())];
							cout<<"Webmode else spill sjobs size before splicing: "<<sjobs.size()<<endl;
							list<Job>::iterator it;
							it = sjobs.begin();
							sjobs.splice(it, jobsForThisWorker);
							cout<<"Webmode else spill sjobs size before splicing: "<<sjobs.size()<<endl;
							
						}
					}
					
				}
				
				
				else{ //it means that there are no nodes booting up - so starting up new nodes and send spilled_jobs to them
					cout<<"Webmode Get slowest job in milliseconds: "<<
					getSlowestJobTime()<< endl;
					
					cout<<"Webmode spilled_jobs "<<
					spilled_over_jobs<< endl;
					
					//calculating the time(in milliseconds) required for spilled over jobs
					long time_for_spilled_jobs = spilled_over_jobs*getSlowestJobTime();
					cout<<"Webmode time_for_spilled_jobs (in milliseconds): "<<
					time_for_spilled_jobs<<
					endl;
					
					double workers_to_be_started = ceil(((double)time_for_spilled_jobs/(CHARGINGTIME) ));
					cout<<"Webmode workers_to_be_started "<<
					workers_to_be_started<<
					endl;
					
					//TODO: for improvement
					// all jobs_per_worker (also in all of above) calculations will result in some excess jobs if jobs_per_worker/num_of_workers doesnt divide perfectly
					// however the algorithm will work since the excess jobs will remain in queuedJobs
					
					int jobs_per_new_worker = ((spilled_over_jobs /workers_to_be_started ) == 0 ? spilled_over_jobs :spilled_over_jobs/workers_to_be_started);
					
					//! if jobs per new worker are less than the workers to be started, then startup only jobs_per_new_worker number of nodes	
					//TODO: Problem: The value 1 is a hack to get this working. Must be improved!
					workers_to_be_started = (jobs_per_new_worker<workers_to_be_started) ? 1 : workers_to_be_started;
					
					stringstream s;
					s<<"Webmode Scheduler will now startup "<<workers_to_be_started<<" new nodes because there are "<<spilled_over_jobs<<" spilled over jobs";
					log->decision(s.str());
					
					//TODO: problem: Handle case where, if the no. of workers to be started is more than the no. of workers in the config file.
					//causes SEG FAULT, if not handled!
					
					list<int> newWorkerIDs = startupNewWorkers(workers_to_be_started);
					cout<<getCurrentTime()<<" Webmode Started "<<newWorkerIDs.size()<<" workers\n";
					cout << "Jobs per new worker is: "<<jobs_per_new_worker<<endl;
					list<int>::iterator n;
					
					//uniformly submitting jobs to the newWorkers
					for(n=newWorkerIDs.begin();n!=newWorkerIDs.end();n++) 
					{
						cout<<getCurrentTime()<<" Webmode new worker created\n";
						cout<<getCurrentTime()<<" queuedJobs.size() 6:"<<queuedJobs.size()<<endl;
						list<Job> jobsForThisWorker = fetchJobsFromQueue(jobs_per_new_worker);
						
						//adding it in map. Will be removed when jobs are scheduled
						cout<<getCurrentTime()<<" Webmode Added list of spilled jobs to worker ID: "<< (*n) <<endl;
						spilledJobsMap[ (*n) ] = jobsForThisWorker;
						
					}
				}
				
			}
			
			
		}
    }
}

//returns the time remaaining in the next charging tick for a worker
long Scheduler::timeTillNextChargingTick(Worker* worker)
{
	long current_worker_time = worker->getTotalCPUTime();
	return  ( (CHARGINGTIME) - (current_worker_time%(CHARGINGTIME) ));
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
				//stopping the worker node
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
	
	
	/*
	 int mapSize = spilledJobsMap.size();
	 int mapcount=0;
	 while(mapcount < mapSize)
	 {
	 map<int, list<Job> >::iterator spilledJobsMap.begin();
	 }*/
	if(spilledJobsMap.size() > 0) {
		cout<<"Webmode SpilledJobsMap size: "<<spilledJobsMap.size()<<endl;
		
		
		
		map<int,list<Job> >::iterator it;
		for(it=spilledJobsMap.begin();it!=spilledJobsMap.end();it++)
		{
			list<Job> jobs = (*it).second;
			int num_jobs = jobs.size();
			int wid = (*it).first;
			Worker *worker = getWorkerObject(wid);
			int wid_copy = wid; //worker->getWorkerID();
			
			cout<< "Worker ID: "<<wid <<endl;
			
			/*sending jobs to this worker*/
			bool successful = worker->submitJobs(jobs);
			if(successful)
			{
				//marking the jobs as started
				markJobsAsStarted(jobs, wid);
				
				//erasing the entry from the map
				spilledJobsMap.erase(wid);
				cout<<"Webmode crashed?\n";
				//it--;
				
				stringstream s;
				s<<getCurrentTime()<<"Webmode Submitted "<<num_jobs<<
				" spilled jobs to worker "<<wid_copy<<endl;
				cout<<getCurrentTime()<<" Webmode Submitted "<<num_jobs<<
				" spilled jobs to worker "<<wid_copy<<endl;
				log->decision(s.str());
				cout<<"Webmode crashed after log decision?\n";
				break;
			}
			else
			{
				cout<<"Webmode crashed in else?\n";
				stringstream s;
				s<<getCurrentTime()<<"Webmode Unable to submit "<<jobs.size()<<
				" spilled jobs to worker "<<wid<<endl;
				log->decision(s.str());		   
			}
			/*sending jobs code ends here*/
			cout<<"\n\nWebmode crashed after if else?\n";
			
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
	
	//DEBUG
	if(num_jobs > (int)queuedJobs.size()) {
		cout<<"Webmode queuedJbs doesnt have this much jobs: Num Jobs"<<num_jobs<<"Queue size: "<<queuedJobs.size()<<endl;
	}
	list<Job> fetchedJobs;
	int jcount=0;
	
	while(jcount<num_jobs)
	{
		cout<<"Inside while loop fetchJobsFromQUeue\n";
		fetchedJobs.push_back(queuedJobs.front());
		//TODO: causing an error. FIX THIS!
		queuedJobs.pop_front();
		jcount++;
	}
	
	return fetchedJobs;
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
		
		
		if(scheduler_mode == "S")
		{
			runSingleTaskScheduler();
		}
		
		else if(scheduler_mode == "W")
		{
			//running the web mode scheduler
			runWebModeScheduler();
			
		}
		
		else if(scheduler_mode == "I")
		{
			runWebModeSchedulerImproved();
		}
		
		else if(scheduler_mode == "R")
		{
			//running the round robin scheduler
			runRoundRobinScheduler();
		}
		//gathering statistics of all workers
		//gatherStatisticsFromAllWorkers();
		
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
  //cout<<"BOATARDE!"<<endl;
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
			//			cout << "foo contains:\n";
			//		  for ( it=taskTimeAverage.begin() ; it != taskTimeAverage.end(); it++ )
		    // cout << (*it).first << " TaskId +--Avg. Time " << (*it).second/1000 << endl;
			
			
			/*
			 * calculating interesting values for WebModeScheduler
			 */
			
		    
			long jobduration = (*i).getEndTime() - (*i).getStartTime();
			calculateAverageJobDuration(jobduration);
			calculateFastestJobTime(jobduration);
			calculateSlowestJobTime(jobduration);
			
			if(getCurrentTime()%10000 == 0)
			cout<<getCurrentTime()<<" Webmode slowest job time: "<<getSlowestJobTime()<<endl;
			
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
