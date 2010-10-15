#include <list>
#include <iostream>

#include "worker.h"

using namespace std;

Worker::Worker(int worker_id) {
  id = worker_id;
  state.current = OFFLINE;
  state.start = 0;
  state.accepting_jobs = false;
  state.time_spent = 0;
  setDefaultProperties();
  current_job = NULL;
}

void Worker::execute() {
  switch (state.current) {
  case INITIALISING: 
    initialise();
    break;
  case COMPUTING:
    compute();
    break;
  case FINALISING:
    finalise();
    break;
  case IDLE:
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

long Worker::getTimeToStart() {
  return properties.time_to_startup;
}

long Worker::getSwappingTime() {
  return properties.swapping_time;
}

long Worker::getInstructionsPerTime() {
  return properties.instructions_per_time;
}

bool Worker::ping() {
  return (state.current == IDLE ||
          state.current == COMPUTING ||
          state.current == INITIALISING);
}

/* Private methods */
bool Worker::startJob() {
  if (current_job == NULL) {
    list<Job>::iterator i;
    i = jobs.begin();
    tmp_current_job = *i;
    current_job = &tmp_current_job;
    jobs.pop_front();
    setState(COMPUTING, true);
    debug("Started job");
    return true;
  }

  return false;
}

void Worker::removeJob() {
  current_job = NULL;
  setState(IDLE, true);
}

void Worker::initialise() {
  if ((currentTime-state.start) < properties.time_to_startup) {
    debug("init");
  } else {
    setState(IDLE, true);
    if (hasMoreWork())
      startJob();
  }
}

void Worker::compute() {
  if (hasMoreWork()) 
    startJob();
  
  if ((currentTime-state.start) < getTotalComputationTime()) {
    debug("computing");      
  } else {
    removeJob();
    // notify Scheduler
  }  
}

void Worker::finalise() {
  debug("finalising");
  setState(OFFLINE, false);
}

void Worker::idle() {
  if (hasMoreWork())
    startJob();
}

bool Worker::hasMoreWork() {
  return (jobs.size() > 0);
}

bool Worker::setState(enum worker_states newstate, bool accept_jobs) {
  state.current = newstate;
  state.start = currentTime;
  state.accepting_jobs = accept_jobs;
  return true;
}

long Worker::getTotalComputationTime() {
  return 20;
}

void Worker::debug(const char *msg) {
  if (DEBUG) 
    cout << "[Worker][Time: " << currentTime << "][WID: " << id << "]"  \
         << "[WS: " << state.current << "] " << msg << endl;
}

void Worker::setDefaultProperties() {
  properties.memory = 512;
  properties.cost_per_hour = 50;
  properties.time_to_startup = 10;
  properties.swapping_time = 5;
  properties.instructions_per_time = 1;
}
