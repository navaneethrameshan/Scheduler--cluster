#include <list>
#include <iostream>

#include "worker.h"

using namespace std;

Worker::Worker(int worker_id, Scheduler *sched) {
  id = worker_id;
  scheduler = sched;
  state.current = OFFLINE;
  state.start = 0;
  state.accepting_jobs = false;
  state.time_spent = 0;
  setDefaultProperties(); // todo: this should be set externally
  current_job = NULL;
  total_execution_time = 0;
  total_cpu_time = 0;

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
      jobs.push_back(*newjob); // should check max size
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
  return (state.current == IDLE || 
          (state.current == COMPUTING &&
           state.accepting_jobs == true)) ;
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
          state.current == INITIALISING);
}

long Worker::getTotalExecutionTime() {
  return total_execution_time/getInstructionsPerTime();
}

long Worker::getTotalCPUTime() {
  return total_cpu_time/getInstructionsPerTime();
}

long Worker::getTotalCost() {
  // assuming time is seconds
  long hours = getTotalCPUTime() / 60;
  // rounding up
  if ((hours % 60) > 0) 
    hours++;

  return hours*getCostPerHour();
}

int Worker::getQueuedJobs() {
  return (int)jobs.size();
}

/* Private methods */
bool Worker::startJob() {
  if (current_job == NULL) {
    list<Job>::iterator i;
    i = jobs.begin();
    tmp_current_job = *i;
    current_job = &tmp_current_job;
    jobs.pop_front();

    logger->debugInt("Starting job", current_job->getJobID());
    state.available_memory = getTotalMemory() - current_job->getMemoryConsumption();

    setState(COMPUTING, true);
    return true;
  }

  return false;
}

bool Worker::swapJob() {
  if (current_job == NULL)
    return false; // no job to swap out

  if ((int)jobs.size() == 0) 
    return false; // no jobs to swap in

  /*
   * save progress of current job
   * take current job, place in end of job list
   * startJob()
   */
  logger->debugInt("Swapping out job", current_job->getJobID());

  int instructions_completed = currentTime-state.start;
  current_job->addInstructionsCompleted(instructions_completed);
  jobs.push_back(tmp_current_job);
  current_job = NULL;
  
  return startJob();
}

void Worker::removeJob() {
  current_job = NULL;
  state.available_memory = getTotalMemory();
  setState(IDLE, true);
}

void Worker::initialise() {
  if ((currentTime-state.start) < properties.time_to_startup) {
    logger->debugInt("Starting worker", getWorkerID());
  } else {
    setState(IDLE, true);
    if (hasMoreWork())
      startJob();
  }
}

void Worker::compute() {
  if (hasMoreWork()) {
    startJob();
  }

  int instructions_completed = currentTime - state.start + 
    current_job->getInstructionsCompleted(); 

  if (instructions_completed < getTotalComputationTime()) {
    // todo: cyclic job swapping here (or spice with intelligence if needed) 
  } else {
    logger->debugInt("Removing job", current_job->getJobID());
    scheduler->notifyJobCompletion(current_job->getJobID()); 
    removeJob();
  }  
}

void Worker::finalise() {
  logger->debugInt("Shutting down worker", getWorkerID());
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

void Worker::setDefaultProperties() {
  properties.memory = 2048;
  properties.cost_per_hour = 50;
  properties.time_to_startup = 0;
  properties.swapping_time = 5;
  properties.instructions_per_time = 1;
}
