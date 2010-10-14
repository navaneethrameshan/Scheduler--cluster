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
  properties.memory = 512;
  properties.cost_per_hour = 50;
  properties.time_to_startup = 10;
  properties.swapping_time = 5;
  properties.instructions_per_time = 1;
}

void Worker::execute() {
  debug("Doing work"); 

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
    break;
  case OFFLINE:
    break;
  }  
}

/* API towards scheduler */
bool Worker::startWorker() {
  state.started = currentTime;
  return setState(INITIALISING, true);
}

bool Worker::stopWorker() {
  state.started = 0;
  return setState(OFFLINE, false);
}

bool Worker::submitJobs(list<Job> newjobs) {
  std::list<Job>::iterator job;
  for (job = newjobs.begin(); job != newjobs.end(); ++job) {
    jobs.push_back(*job); // should check max size
  }
  return true;
}

enum worker_states Worker::getState() {
  return state.current;
}

int Worker::getAvailableMemory() {
  return state.available_memory;
}

bool Worker::getAcceptingJobs() {
  return state.accepting_jobs;
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

/* Private methods */
void Worker::initialise() {
  if ((currentTime-state.start) < properties.time_to_startup) {
    debug("init");
  } else
    setState(COMPUTING); 
}

void Worker::compute() {
  if ((currentTime-state.start) < getTotalComputationTime())
    debug("computing");
  else
    setState(IDLE);
}

void Worker::finalise() {
  debug("finalising");
  setState(OFFLINE);
}

bool Worker::setState(enum worker_states newstate) {
  return setState(newstate, true);
}

bool Worker::setState(enum worker_states newstate, bool accept_jobs) {
  state.current = newstate;
  state.start = currentTime;
  return true;
}

long Worker::getTotalComputationTime() {
  return 20;
}

void Worker::debug(const char *msg) {
  if (DEBUG) 
    cout << "[Time: " << currentTime << "][WID: " << id << "]"  \
         << "[WS: " << state.current << "] " << msg << endl;
}
