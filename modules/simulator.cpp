#include <list>
#include <iostream>
#include "worker.h"
#include "simulator.h"

using namespace std;

Simulator::Simulator() {
  clocktime = 0;
  stopping = false;
}

void Simulator::init() {
  // set scheduler
  // set task generator
  // initialise worker list
  cout << "\n";
  debug("Starting simulator");

  Worker *w1 = new Worker(1);
  workers.push_front(w1);
  
  Worker *w2 = new Worker(2);
  workers.push_front(w2);
  
}

void Simulator::execute() {
  while (true) {
    if (stopping || clocktime == 100)
      break;

    debug("Executing");

    // execute tg + scheduler

    // iterate all workers
    list<Worker *>::iterator worker;
    for (worker = workers.begin(); worker != workers.end(); ++worker) {
      (*worker)->execute();
    }

    tick();
  }

  cleanUp();
}

void Simulator::stop() {
  stopping = true;
}

void Simulator::tick() {
  clocktime += 1;
}

bool Simulator::cleanUp() {
  debug("Stopping.");
  return true;
}

void Simulator::debug(const char* msg) {
  if (DEBUG)
    cout << "[" << clocktime << "] " << msg << "\n";
}
