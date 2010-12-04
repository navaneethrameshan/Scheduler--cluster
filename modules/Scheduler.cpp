#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <list>
#include <sstream>
#include "Scheduler.h"

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
  /*cout<<"[Scheduler] Starting with following configuration"<<endl
      <<"[Scheduler] scheduler_mode "<<scheduler_mode<<endl
      <<"[Scheduler] scheduling_interval "<<scheduling_interval<<endl;
  */
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

  //! If required, the Simulator can submit new worker nodes to Scheduler during the Simulation
  int Scheduler::submitWorkers(list<Worker *> workers)
  {
    	list<Worker *>::iterator i;
	for(i=workers.begin(); i!=workers.end();++i)
	  {
	this->workers.push_back(*i);
	  }
	return 0;
  }

/*
unsigned int getNumberOfUsableWorkerNodes(List<Worker *> workers)
{
  list<Worker *>::iterator i;
  for(i=workers.begin();i!=workers.end(),++i)
    {
      if( (*i)->getState() == worker_states.OFFLINE || (*i)->getState() == worker_states.COMPUTING ||(*i)->getState() == worker_states.IDLE )
	{
	  num_of_usable_worker_nodes++;
	}
    }
  //Outputting
  count<<"Number of Usable Worker Nodes: "<<num_of_usable_worker_nodes<<endl;
}
*/

  //! Runs the scheduler (e.g. start Worker nodes, stop Worker nodes, submitJobs) - will be executed at each clock tick by Simulator


int Scheduler::runScheduler()
{ 
  
      milliseconds++;
  
  if((milliseconds)%scheduling_interval_for_clock == 0 || milliseconds == 1) 
    {
  
      if(isFirstTime == true ) {
	j = workers.begin();
	isFirstTime = false;
      }
      
      
  

    if( (int)queuedJobs.size() == 0 && (int)this->runningJobs.size() == 0 )
      {
	log->decision("Nothing to do, so chilling!");
	return 0;
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
    } //end of clocktick if

  return 0; //returning successful exit everytime (for the time being)
}
    

//! A Worker node will notify the Scheduler when a job finishes its execution
int Scheduler::notifyJobCompletion(unsigned int job_id)
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
	  }
      }
    return 0; //returning 0 always (for the time being)

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
  
}

