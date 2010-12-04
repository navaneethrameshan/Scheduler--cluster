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
  current_job = NULL;
  total_execution_time = 0;
  total_cpu_time = 0;
  job_carry_over = 0;

  logger = Logger::getLogger();
}

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
bool Worker::startWorker() {
  state.started = currentTime;
  return setState(INITIALISING, false);
}

bool Worker::stopWorker() {
  state.started = 0;
  return setState(FINALISING, false);
}

/* currently accepts all jobs even if all wont fit */ 
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

enum worker_states Worker::getState() {
  return state.current;
}

int Worker::getAvailableMemory() {
  return state.available_memory;
}

bool Worker::isAcceptingJobs() {
  return state.accepting_jobs;
}

int Worker::getTotalMemory() {
  return properties.memory;
}

float Worker::getCostPerHour() {
  return properties.cost_per_hour;
}

long Worker::getInstructionsPerTime() {
  return properties.instructions_per_time;
}

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

long Worker::getTotalExecutionTime() {
  return total_execution_time;
}

long Worker::getTotalCPUTime() {
  return total_cpu_time;
}

double Worker::getAverageResponseTime() {
  double avg = 0;
  int number_of_jobs = 0;
  
  list<Job>::iterator job;
  for (job = jobs.begin(); job != jobs.end(); ++job) {
    avg += currentTime - (*job).getStartedTime();
  }

  if (current_job != NULL) {
    avg += currentTime - current_job->getStartedTime();
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

bool Worker::cancelJob(unsigned int jobId) {
  if (current_job->getJobID() == jobId) {
    removeJob();
    return true;
  }

  list<Job>::iterator job;
  for (job = jobs.begin(); job != jobs.end(); ++job) {
    if ((*job).getJobID() == jobId) {
      jobs.erase(job);
      job--;
      return true;
    }
  }

  return false; // no such job id on this worker
}

/* ============= Private methods ============== */
bool Worker::startJob() {
  if (current_job == NULL) {
    list<Job>::iterator i;
    i = jobs.begin();
    tmp_current_job = *i;
    current_job = &tmp_current_job;
    jobs.pop_front();

    logger->debugInt("Starting job", current_job->getJobID());
    state.available_memory = getTotalMemory() - current_job->getMemoryConsumption();

    // carryover from previous job
    current_job->addInstructionsCompleted(job_carry_over);
    job_carry_over = 0;

    /* Todo:
     * Try to start job, if it exceeds available memory
     * notify scheduler that it is too big. Ask to transfer/cancel.
     */

    setState(COMPUTING, true);
    return true;
  }

  return false;
}

bool Worker::swapJob() {
  if (getState() == SWAPPING) 
    return false; // already swapping

  if (current_job == NULL)
    return false; // no job to swap out

  if ((int)jobs.size() == 0) 
    return false; // no jobs to swap in

  logger->debugInt("Swapping out job", current_job->getJobID());

  int instructions_completed = currentTime - state.start;
  tmp_job_size = current_job->getMemoryConsumption();
  current_job->addInstructionsCompleted(instructions_completed);
  current_job->increaseSwapCount();
  jobs.push_back(tmp_current_job);
  current_job = NULL;

  setState(SWAPPING, false);
  
  return true;
}

void Worker::removeJob() {
  current_job = NULL;
  state.available_memory = getTotalMemory();
  setState(IDLE, true);
}

void Worker::initialise() {
  if ((currentTime-state.start) == properties.time_to_startup) {
    logger->debugInt("Worker with ID started", getWorkerID());
    setState(IDLE, true);
    if (hasMoreWork())
      startJob();
  }
}

void Worker::compute() {
  int instructions_completed;
  if (hasMoreWork()) {
    startJob();
  }  

  if (current_job != NULL) {
    instructions_completed = (currentTime - state.start) *
      properties.instructions_per_time + current_job->getInstructionsCompleted(); 

    if ((job_carry_over = instructions_completed - getTotalComputationTime()) >= 0) {
      logger->workerInt("Removing job", current_job->getJobID());
      scheduler->notifyJobCompletion(current_job->getJobID(), id); 
      removeJob();
    }  

    if ((instructions_completed % 500) == 0) {
      swapJob();            
    }
  }
}

void Worker::swap() {
  if ((currentTime-state.start) >= calculateSwappingTime()) {
    setState(IDLE, true);
    logger->workerInt("Swap completed on", getWorkerID());
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
  return ((int)jobs.size() > 0);
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

unsigned int Worker::calculateSwappingTime() {
  int swaptime = tmp_job_size / 1024 * properties.swapping_time;
  return swaptime;
}

void Worker::setProperties(WORKER_PROPERTIES *props) {
  properties.memory = props->memory;
  properties.cost_per_hour = props->cost_per_hour;
  properties.time_to_startup = props->time_to_startup;
  properties.swapping_time = props->swapping_time;
  properties.instructions_per_time = props->instructions_per_time;
  properties.quantum = props->quantum;
}
