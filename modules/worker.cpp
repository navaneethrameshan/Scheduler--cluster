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

void Worker::execute() {
  logger->debugInt("State is", state.current);

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
/* 
first swap between all jobs in memory
if there are elements in the hdd, try to start job
if start job failed, move largest job in mem to hdd (pay swapping) 
loop 
  retry start job, if failed move largest job in mem to hdd (pay swapping)

 */

void Worker::debugJobs() {
  list<Job>::iterator job;
  for (job = ram.begin(); job != ram.end(); ++job) {
    cout << "In ram: " << (*job).getJobID() << endl;
  }

  for (job = jobs.begin(); job != jobs.end(); ++job) {
    cout << "In hdd: " << (*job).getJobID() << endl;
  }
  
}

bool Worker::startJob() {
  // test start jobs and move as many as possible into memory
  bool result;

  while (moveJobToMemory());

  debugJobs();
  
  if (activateJob()) {
    result = true;
    logger->debugInt("Activated job", current_job->getJobID());
  }
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
    logger->debugInt("MEMORY", state.available_memory);
    if ((state.available_memory - jobs.front().getMemoryConsumption()) >= 0) {
      logger->debug("ADDING JOB TO MEM since it fits... cheating a little");
      list<Job>::iterator i;
      i = jobs.begin();
      ram.push_back(*i);
      jobs.pop_front();
      logger->debugInt("added job", ram.front().getMemoryConsumption());
      state.available_memory -= jobs.front().getMemoryConsumption();
      // add swap cost
      return true;
    } 
  }
  return false;
}

bool Worker::activateJob() {
  if (current_job != NULL)
    return false;

  // take from mem queue, although it is still considered being in memory
  list<Job>::iterator i;
  i = ram.begin();
  tmp_current_job = *i;
  current_job = &tmp_current_job;
  ram.pop_front();
  
  time_to_swap = 0;

  logger->debugInt("Time to swap", calculateSwappingTime(current_job));

  setState(COMPUTING, true);
  return true;
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
  
  setState(SWAPPING, false);
  
  return true;
}

void Worker::removeJob() {
  state.available_memory += current_job->getMemoryConsumption();
  //ram.remove(tmp_current_job);
  current_job = NULL;
  job_carry_over = 0;
  setState(IDLE, true);
}

void Worker::initialise() {
  if ((currentTime-state.start) == properties.time_to_startup) {
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
    logger->debugInt("IN COMPUTE", current_job->getJobID());
    logger->debugInt("IN COMPUTE compl", current_job->getInstructionsCompleted());
    logger->debugInt("IN COMPUTE instr", properties.instructions_per_time);
    logger->debugInt("IN COMPUTE tot", current_job->getNumberOfInstructions());

    job_carry_over = 
      current_job->addInstructionsCompleted(properties.instructions_per_time + 
                                            job_carry_over);
    logger->debugInt("IN COMPUTE carry_over", job_carry_over);

    if ((getTotalComputationTime() - current_job->getInstructionsCompleted()) <= 0) {
      logger->workerInt("Removing job", current_job->getJobID());
      scheduler->notifyJobCompletion(current_job->getJobID()); 
      removeJob();
      // possibly return here
    }

    logger->debugInt("CURRENTTIME", currentTime);
    if ((currentTime % 5) == 0) {
      logger->debug("TIME FOR SWAPPING");
      swapJob();            
    }
  }
}

void Worker::swap() {
  logger->debugInt("Swapping time is", time_to_swap);
  logger->debugInt("State.start", state.start);
  if ((currentTime-state.start) == time_to_swap) {
    startJob();
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
  return ((int) ram.size() + (int)jobs.size() > 0);
}

bool Worker::setState(enum worker_states newstate, bool accept_jobs) {
  logger->debugInt("CURRENT STATE is", state.current);
  state.current = newstate;
  state.start = currentTime;
  state.accepting_jobs = accept_jobs;
  logger->debugInt("CHANGED STATE to", state.current);
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

void Worker::setProperties(WORKER_PROPERTIES *props) {
  properties.memory = props->memory;
  properties.cost_per_hour = props->cost_per_hour;
  properties.time_to_startup = props->time_to_startup;
  properties.swapping_time = props->swapping_time;
  properties.instructions_per_time = props->instructions_per_time;
  properties.quantum = props->quantum;
}
