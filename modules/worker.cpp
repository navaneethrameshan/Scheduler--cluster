#include "worker.h"
#include <iostream>

using namespace std;

Worker::Worker(int worker_id) {
  id = worker_id;
  state.current = INITIALISING;
  state.tick = 0;
}

void Worker::initialise() {
  if (state.tick < 10)
    cout << "init in " << id << "\n";
  else
    setState(COMPUTING); 
}

void Worker::compute() {
  cout << "computing\n";
}

void Worker::finalise() {
  cout << "finalising\n";
}

void Worker::execute() {
  cout << "Doing work in " << id << "\n"; 

  // possible to check for global tick manipulation... queue ticks?

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
  }

  tick();
}

void Worker::tick() {
  state.tick += 1;
}

bool Worker::setState(enum states newstate) {
  state.current = newstate;
  state.tick = 0;
  return true;
}
