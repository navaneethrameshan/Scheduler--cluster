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

  //! default constructor - reads the configuration from modules/scheduler.conf
Scheduler::Scheduler() 
{
  log = Logger::getLogger();
  cout << "KLASJDFLKSADJFKLSDJALSKDJF";
  isFirstTime = true;
  milliseconds = 0;
  string line;
  ifstream infile;
  string values[10];
  int i=0;
  infile.open ("modules/scheduler.conf");

  while(!infile.eof()) // To get you all the lines.
    {
      getline(infile,line); // Saves the line in STRING.
      if( (line.find('#') != 0) ) //this will ignore the lines having a '#' character 
	{
	  values[i] = line;
	  i++;
	}
    }
  infile.close();

  
  scheduler_mode = values[0]; //W or S
  scheduling_interval = strtod(values[1].c_str(), NULL); //0.01 to 1 seconds atoi(strNumber.c_str());
  scheduling_interval_for_clock = (unsigned short int)floor((1/scheduling_interval)+0.5);
  worker_node_speed = strtod(values[2].c_str(), NULL); //200 - 400 instructions per second
  worker_node_memory = strtod(values[3].c_str(), NULL); //2-8 GB
  worker_node_swapping_cost = strtod(values[4].c_str(), NULL); //2-10 instructions per GB
  worker_quantum = strtod(values[5].c_str(), NULL); //0.01 to 0.5 seconds
  worker_node_startup_time = strtod(values[6].c_str(), NULL); //120-600 seconds
  worker_node_sched_notif_time = strtod(values[7].c_str(), NULL); //1-5 instructions
  worker_node_cost = strtod(values[8].c_str(), NULL); //in euros/hour

  cout<<"[Scheduler] Starting with following configuration"<<endl
      <<"[Scheduler] scheduler_mode "<<scheduler_mode<<endl
      <<"[Scheduler] scheduling_interval "<<scheduling_interval<<endl
      <<"[Scheduler] worker_node_speed "<<worker_node_speed<<endl
      <<"[Scheduler] worker_node_memory "<<worker_node_memory<<endl
      <<"[Scheduler] worker_node_swapping_cost "<<worker_node_swapping_cost<<endl
      <<"[Scheduler] worker_quantum "<<worker_quantum<<endl
      <<"[Scheduler] worker_node_startup_time "<<worker_node_startup_time<<endl
      <<"[Scheduler] worker_node_sched_notif_time "<<worker_node_sched_notif_time<<endl
      <<"[Scheduler] worker_node_cost "<<worker_node_cost
      <<endl;

  queuedJobs.clear(); //why do we need this?
 
}

//! This constructor has been deprecated. Please use Scheduler()
Scheduler::Scheduler(string scheduler_mode, float scheduling_interval)
{
  this->scheduler_mode = scheduler_mode;
  this->scheduling_interval = scheduling_interval;
  milliseconds = 0;
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

long Scheduler::getCurrentTime() 
{
  return milliseconds;
}

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
    } //end of clocktick if

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

  workerStats.front()->print();
  
}

