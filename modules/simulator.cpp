#include <list>
#include <iostream>

#include "simulator.h"

using namespace std;

Simulator::Simulator() {
  stopping = false;
}

void Simulator::init() {
  // set scheduler
  // set task generator
  // initialise worker list
  cout << "\n";
  debug("Starting simulator");

  // this will be read from file
  Worker *w1 = new Worker(1);
  workers.push_front(w1);
  

  Worker *w2 = new Worker(2);
  if (w2->startWorker()) 
    cout << "Started worker 2" << endl;
  workers.push_front(w2);
  
}

void Simulator::execute() {
  while (true) {
    if (stopping || currentTime == 50)
      break;

    debug("Executing");

    // execute tg + scheduler

    // iterate all workers
    list<Worker *>::iterator worker;
    for (worker = workers.begin(); worker != workers.end(); ++worker) {
      (*worker)->execute();
    }

    currentTime++;
  }

  cleanUp();
}

void Simulator::stop() {
  stopping = true;
}

bool Simulator::cleanUp() {
  debug("Stopping.");
  return true;
}

void Simulator::debug(const char* msg) {
  if (DEBUG)
    cout << "[" << currentTime << "] " << msg << "\n";
}
