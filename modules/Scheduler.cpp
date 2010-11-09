#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <list>
#include "Scheduler.h"

using namespace std;
 
  // default constructor - reads the configuration from modules/scheduler.conf
Scheduler::Scheduler() 
{
  //Scheduler(/*"SINGLE_TASK"*/0, 0, 0);
  
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
	  cout<<values[i]<<endl;
	  i++;
	}
    }
  infile.close();

  
  scheduler_mode = values[0]; //W or S
  scheduling_interval = strtod(values[1].c_str(), NULL); //0.01 to 1 seconds atoi(strNumber.c_str());
  worker_node_speed = strtod(values[2].c_str(), NULL); //200 - 400 instructions per second
  worker_node_memory = strtod(values[3].c_str(), NULL); //2-8 GB
  worker_node_swapping_cost = strtod(values[4].c_str(), NULL); //2-10 instructions per GB
  worker_quantum = strtod(values[5].c_str(), NULL); //0.01 to 0.5 seconds
  worker_node_startup_time = strtod(values[6].c_str(), NULL); //120-600 seconds
  worker_node_sched_notif_time = strtod(values[7].c_str(), NULL); //1-5 instructions
  worker_node_cost = strtod(values[8].c_str(), NULL); //in euros/hour

  cout<<"[Scheduler starting]"<<endl
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

// constructor
Scheduler::Scheduler(string scheduler_mode, float scheduling_interval)
{
  this->scheduler_mode = scheduler_mode;
  this->scheduling_interval = scheduling_interval;
  queuedJobs.clear ();
}
  
  //this function will start a worker and return its worker_id

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

//this function will stop a worker and returns 0 on successful execution
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
  
  
  // TaskGenerator will submit jobs to the scheduler using this function
  int Scheduler::submitJobs(list<Job > jobs)
  {
	list<Job >::iterator i;
	for(i=jobs.begin(); i!=jobs.end();++i)
	  {
	  cout<< "\nSCHED:: PUSHED JOB TO QUEUED JOBS"<<queuedJobs.size()<<"\n";
	queuedJobs.push_back(*i);	
	  }
	return 0;
      
  }

  // if required, the Simulator can submit new worker nodes to Scheduler during the Simulation
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

  //Runs the scheduler (e.g. start Worker nodes, stop Worker nodes, submitJobs) - will be executed at each clock tick by Simulator
  int Scheduler::runScheduler()
  { 
    	
    if( (int)queuedJobs.size() == 0 && (int)this->runningJobs.size() == 0 )
      {
	//Nothing to do, so chilling!
	cout<<"Nothing to do, so chilling!"<<endl;
	return 0;
      }
    else 
      {
	if( (int)queuedJobs.size() > 0) 
	  {
			//To test Navaneeth
list<Job >::iterator k;
 for(k=queuedJobs.begin();k!=queuedJobs.end();++k)
	      {
		(*k).show();
		}
//To test end	   
		
	    //int number_of_queued_jobs = (int)this->queuedJobs.size();
	    //int number_of_usable_worker_nodes = getNumberOfUsableNodes(this->workers); //Commented out - not required
	    
	    //start nodes for each job in queue
	    list<Job >::iterator i;
	    //if(number_of_queued_jobs <= number_of_usable_worker_nodes ) //Commented out because condition is not required
	      for(i=queuedJobs.begin();i!=queuedJobs.end();++i)
	      {
		list<Worker *>::iterator j;
		for(j=workers.begin();j!=workers.end();++j)
		  {
		    /* if( (*j)->getState() == worker_states.COMPUTING || (*j)->getState() == worker_states.IDLE || (*j)->getState() == worker_states.OFFLINE )*/
		    	cout<< "\n STATE OF WORKER "<< (*j)->getState() <<"\n";
 if( (*j)->getState() == COMPUTING || (*j)->getState() == IDLE || (*j)->getState() == OFFLINE )
		      {
			list<Job> jobs_to_submit;
			jobs_to_submit.push_back(*i);	
			//(*j)->startWorker(); //starting a worker node
			if( (*j)->submitJobs(jobs_to_submit) == true ) //submitting Job to the worker node
			  {
			    runningJobs.push_back(*i); //adding the job to runningJobs
			    cout<<"Job "<<(*i).getJobID()<<" submitted to "<<(*j)->getWorkerID()<<endl; //Outputting
			    queuedJobs.erase(i); //erasing the Job from the queuedJobs
			    --i; //Added during Testing (Navaneeth and Marcus)
			    break; //breaking the workers loop in any case and moving on with next job
			  }
			else
			  {
			    continue;
			  }

		      }
		  }
		}

	  }
      }

    return 0; //returning successful exit everytime (for the time being)
  }
    

  // a Worker node will notify the Scheduler when a job finishes its execution
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

  // outputs the current state of a Scheduler object (can be static also; will be decided later on)
  void Scheduler::print()
  {
    cout<<"Scheduler Mode: "<<scheduler_mode<<endl;
    cout<<"Scheduling Interval: "<<scheduling_interval<<endl;
    cout<<"Number of queuedJobs: "<<(int)queuedJobs.size()<<endl;
    cout<<"Number of runningJobs: "<<(int)runningJobs.size()<<endl;
    cout<<"Number of completedJobs: "<<(int)completedJobs.size()<<endl;
    cout<<"Total Number of workers: "<<(int)workers.size()<<endl;

    list<Worker *>::iterator i;
    int offline_workers_count = 0;
    int idle_workers_count = 0;
    int computing_workers_count = 0;

    for(i=workers.begin();i!=workers.end();++i)
      {
	if( (*i)->getState() == OFFLINE  )
	  offline_workers_count++;
	if( (*i)->getState() == IDLE  )
	  idle_workers_count++;
	if( (*i)->getState() == COMPUTING  )
	  computing_workers_count++;
      }

  cout<<"-Number of OFFLINE workers: "<<offline_workers_count<<endl;
  cout<<"-Number of IDLE workers: "<<idle_workers_count<<endl;
  cout<<"-Number of COMPUTING workers: "<<computing_workers_count<<endl;

  }

