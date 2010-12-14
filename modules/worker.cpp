#include <list>
#include <iostream>

#include "worker.h"

using namespace std;

Worker::Worker(int worker_id, WORKER_PROPERTIES *props, Scheduler *sched) {
  id = worker_id;
  scheduler = sched;
  state.current = OFFLINE;
  state.start = 0;
  state.accepting_jobs = false;
  state.time_spent = 0;
  setProperties(props);
  state.available_memory = properties.memory; 
  current_job = NULL;
  total_execution_time = 0;
  total_cpu_time = 0;
  job_carry_over = 0;

  logger = Logger::getLogger();
}

/*! Executed by the simulator. Checks the worker's current state and 
performs all the work to be done in this time instant. */ 
void Worker::execute() {
  switch (state.current) {
  case INITIALISING: 
    increaseExecutionTime();
    increaseCPUTime();
    initialise();
    break;
  case COMPUTING:
    increaseExecutionTime();
    increaseCPUTime();
    compute();
    break;
  case SWAPPING: 
    increaseExecutionTime();
    increaseCPUTime();
    swap();
    break;
  case FINALISING:
    increaseExecutionTime();
    increaseCPUTime();
    finalise();
    break;
  case IDLE:
    increaseCPUTime();
    idle();
    break;
  case OFFLINE:
    break;
  }  
}

/* API towards scheduler */

/*! Used by the scheduler to start a new worker node. It wont be usable until 
it moves to state IDLE which happens only after a specified (see configuration 
file) time.  */
bool Worker::startWorker() {
  state.started = currentTime;
  return setState(INITIALISING, false);
}

/*! Sets the worker state to OFFLINE after a shutdown period */
bool Worker::stopWorker() {
  state.started = 0;
  return setState(FINALISING, false);
}

/*! Multiple jobs can be sumbitted to a worker when a list is passed. This is used by the scheduler. It returns false in case it cannot accept the job. */
bool Worker::submitJobs(list<Job> newjobs) {
  if (isAcceptingJobs()) {
    list<Job>::iterator newjob;
    for (newjob = newjobs.begin(); newjob != newjobs.end(); ++newjob) {
      logger->workerInt("Job ID received", (*newjob).getJobID());
      (*newjob).setStartedTime();
      jobs.push_back(*newjob); // todo: should check max size
    }
    return true;
  } else 
    return false;
}

/*! Retrieve worker's current state */
enum worker_states Worker::getState() {
  return state.current;
}

/*! Retrieve worker's available memory, based on jobs currently stored in memory. */
int Worker::getAvailableMemory() {
  return state.available_memory;
}

/*! Check whether a worker is accepting jobs or not */
bool Worker::isAcceptingJobs() {
  return state.accepting_jobs;
}

/*! How much memory does a worker have in total. This value never changes 
  during exectution.  */
int Worker::getTotalMemory() {
  return properties.memory;
}

/*! How much does a worker cost per hour. This value never changes during 
   execution. */
float Worker::getCostPerHour() {
  return properties.cost_per_hour;
}

/*! How many instructions is a worker able to perform per ms. Never changes 
   during an execution. */
float Worker::getInstructionsPerTime() {
  return (float)properties.instructions_per_time/1000;
}

/*! Returns the worker ID */ 
unsigned int Worker::getWorkerID()
{
  return id;
}

bool Worker::ping() {
  return (state.current == IDLE ||
          state.current == COMPUTING ||
          state.current == SWAPPING || 
          state.current == INITIALISING);
}

/*! Get total time spent doing useful work since machine started. In ms. */
long Worker::getTotalExecutionTime() {
  return total_execution_time;
}

/*! Get total execution time since worker started. */
long Worker::getTotalCPUTime() {
  return total_cpu_time;
}


double Worker::getAverageResponseTime() {
  double avg = 1;
  int number_of_jobs = 0;
  
  list<Job>::iterator job;
  for (job = jobs.begin(); job != jobs.end(); ++job) {
    avg += (currentTime - (*job).getStartedTime());
    number_of_jobs++;
  }

  for (job = ram.begin(); job != ram.end(); ++job) {
    avg += (currentTime - (*job).getStartedTime());
    number_of_jobs++;
  }

  if (current_job != NULL) {
    avg += (currentTime - current_job->getStartedTime());
    number_of_jobs++;
  }

  return (number_of_jobs > 0 ? avg/number_of_jobs : avg);
}

float Worker::getTotalCost() {
  // assuming time is milliseconds
  int s = getTotalCPUTime() / 1000; // seconds
  int m = s / 60; // minutes
  int h = m / 60; // hours;

  // rounding up
  if (h == 0 && m == 0 && s >= 0) 
    h++;

  if ((m % 60) > 0) 
    h++;

  return h*getCostPerHour();
}

int Worker::getQueuedJobs() {
  return (int)jobs.size();
}

bool Worker::cancelJob(unsigned int taskId, unsigned int jobId) {
  if ((current_job->getJobID() == jobId) && (current_job->getTaskID() == taskId)) {
    removeJob();
    return true;
  }

  list<Job>::iterator job;
  for (job = jobs.begin(); job != jobs.end(); ++job) {
    if (((*job).getJobID() == jobId) && (*job).getTaskID() == taskId) {
      jobs.erase(job);
      job--;
      return true;
    }
  }

  return false; // no such job id on this worker
}

std::map<long,int> Worker::getCompletionTimes(int windowSize) {
  return job_completion;
}

/* ============= Private methods ============== */
void Worker::debugJobs() {
  list<Job>::iterator job;
  for (job = ram.begin(); job != ram.end(); ++job) {
    logger->debugInt("In ram", (*job).getJobID());
  }

  for (job = jobs.begin(); job != jobs.end(); ++job) {
    logger->debugInt("In hdd", (*job).getJobID());
  }
  
}

bool Worker::startJob() {
  bool result;

  //  while (moveJobToMemory());
  moveJobToMemory();

  if (activateJob()) {
    result = true;
    logger->workerInt("Activated job", current_job->getJobID());
  }

  //debugJobs();

  return result;
}

bool Worker::moveLargestJobFromMemoryToHdd() {
  if ((int)ram.size() == 0) 
    return false;

  list<Job>::iterator largest = ram.begin();
  list<Job>::iterator job;
  for (job = ram.begin(); job != ram.end(); ++job) {
    if ((*job).getMemoryConsumption() > (*largest).getMemoryConsumption())
      largest = job;
  }

  jobs.push_back(*largest);
  ram.erase(largest);
  return true;
}

bool Worker::moveJobToMemory() {
  if ((int)jobs.size() > 0) {
    if ((state.available_memory - jobs.front().getMemoryConsumption()) >= 0) {
      list<Job>::iterator i;
      i = jobs.begin();
      ram.push_back(*i);

      state.available_memory -= (*i).getMemoryConsumption();

      // add swap cost
      job_carry_over -= calculateSwappingTime(&(*i));

      jobs.pop_front();
      return true;
    } 
  }
  return false;
}

bool Worker::activateJob() {
  if (current_job != NULL)
    return false;

  // take from mem queue, although it is still considered being in memory
  if ((int) ram.size() > 0) {
    list<Job>::iterator i;
    i = ram.begin();
    tmp_current_job = *i;
    current_job = &tmp_current_job;
    ram.pop_front();
    time_to_swap = 0; 
    setState(COMPUTING, true);
    return true;
  } else {
    setState(IDLE,true);
    return false;
  }

}

bool Worker::swapInMemory() {
  // push back current job
  tmp_job_size = current_job->getMemoryConsumption();
  current_job->increaseSwapCount();
  ram.push_back(tmp_current_job);
  current_job = NULL;
  return true;
}

bool Worker::swapJob() {
  if (getState() == SWAPPING) 
    return false; // already swapping

  if (current_job == NULL)
    return false; // no job to swap out

  if ((int)jobs.size() == 0) 
    return swapInMemory(); 

  state.available_memory += current_job->getMemoryConsumption();

  tmp_job_size = current_job->getMemoryConsumption();
  current_job->increaseSwapCount();
  time_to_swap = calculateSwappingTime(current_job);
  jobs.push_back(tmp_current_job);
  current_job = NULL;
  
  setState(SWAPPING, true);
  
  return true;
}

void Worker::removeJob() {
  state.available_memory += current_job->getMemoryConsumption();
  job_completion[currentTime] = (currentTime - current_job->getStartedTime());
  current_job = NULL;
  job_carry_over = 0;
  setState(IDLE, true);
}

void Worker::initialise() {
  if ((currentTime-state.start) == properties.time_to_startup*1000) {
    logger->workerInt("Started Worker (ID)", getWorkerID());
    setState(IDLE, true);
    if (hasMoreWork()) {
      startJob();
    }
  }
}

void Worker::compute() {
  if (hasMoreWork()) {
    startJob();
  }  

  if (current_job != NULL) {

    float completed = getInstructionsPerTime() + job_carry_over;
    job_carry_over = 
      current_job->addInstructionsCompleted(completed);

    if ((getTotalComputationTime() - current_job->getInstructionsCompleted()) <= 0) {
      logger->workerInt("Removing job", current_job->getJobID());
      job_carry_over -= properties.notification_time;
      cout<<"MARCUS Before notifyJob in Worker"<<endl;
      scheduler->notifyJobCompletion(current_job->getTaskID(), 
                                     current_job->getJobID(), id); 
      removeJob();
    }

	  
    if ((currentTime % properties.quantum) == 0) {
      if (current_job == NULL) {
        logger->debug("No job to swap out, starting new");
        startJob();
      } else {
        logger->debugInt("swapping out", current_job->getJobID());
        swapJob();    
      }
    }
  }
}

void Worker::swap() {
  if ((currentTime-state.start) == time_to_swap) {
    logger->debugInt("Swap completed on", getWorkerID());
    startJob();
  }
}

void Worker::finalise() {
  logger->workerInt("Shutting down worker", getWorkerID());
  setState(OFFLINE, false);
}

void Worker::idle() {
  if (hasMoreWork())
    startJob();
}

bool Worker::hasMoreWork() {
  return ((int) ram.size() + (int)jobs.size() > 0);
}

bool Worker::setState(enum worker_states newstate, bool accept_jobs) {
  state.current = newstate;
  state.start = currentTime;
  state.accepting_jobs = accept_jobs;
  return true;
}

long Worker::getTotalComputationTime() {
  return (int)current_job->getNumberOfInstructions(); 
}

void Worker::increaseExecutionTime() {
  total_execution_time++;
}

void Worker::increaseCPUTime() {
  total_cpu_time++;
}

unsigned int Worker::calculateSwappingTime(Job *job) {
  unsigned int swaptime = job->getMemoryConsumption() * 
    properties.swapping_time;
  if (swaptime % 1024 > 0)
    swaptime++;
  return swaptime / 1024;
}

std::map<Job *, int> Worker::getJobFootprints() {
  map<Job *, int> footprint; 
  list<Job>::iterator i; 
  for (i = ram.begin(); i != ram.end(); ++i) {
    footprint[&(*i)] = (*i).getMemoryConsumption();
  }
  if (current_job != NULL) {
    footprint[current_job] = current_job->getMemoryConsumption();
  }
  return footprint;
}

void Worker::setProperties(WORKER_PROPERTIES *props) {
  properties.memory = props->memory;
  properties.cost_per_hour = props->cost_per_hour;
  properties.time_to_startup = props->time_to_startup;
  properties.swapping_time = props->swapping_time;
  properties.instructions_per_time = props->instructions_per_time;
  properties.quantum = props->quantum;
  properties.notification_time = props->notification_time;
}
