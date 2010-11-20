#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "logger.h"

using namespace std;

extern unsigned long currentTime;

bool Logger::loggerFlag = false;
Logger* Logger::loggerInstance = NULL;

/* private methods */
Logger::Logger() {
  logFile.open("cloud.log");
}

void Logger::write(string entry) {
  logFile << entry << endl;
#ifdef DEBUG
  cout << entry << endl;
#endif
}

/* public methods */
Logger* Logger::getLogger() {
  if (!loggerFlag) { 
    loggerInstance = new Logger;
    loggerFlag = true;
  }

  return loggerInstance;     
}

/* for the scheduler */
void Logger::status(string mode, float interval, int queue, int running,
                    int completed) {
  stringstream entry; 
  entry << "[SCHEDULER] ("<<currentTime<<") Mode: " << mode << " Interval: " << interval
        << " Queue: " << queue << " Running: " << running
        << " Completed: " << completed;
  write(entry.str());
}

void Logger::decision(string decision) {
  debug("SCHEDULER", decision);
}

/* for the workers */
void Logger::workerInt(string debugString, int value) {
  debugInt("WORKER", debugString, value);
}

/* for simulator */ 
void Logger::workerAverage(double avg_response_time, float cost,
                           int offline, int idle, int computing, int jobs) {
  stringstream entry;
  entry << "-" << currentTime << "\t" 
        << avg_response_time << "\t"
        << cost << "\t"
        << offline << "\t"
        << idle << "\t"
        << computing << "\t"
        << jobs;
  write(entry.str());
}

void Logger::totals(long exec, long cpu, float cost) {
  stringstream entry;
  entry << "[SIMULATOR] CPU: " << cpu
        << " EXECUTION: " << exec
        << " COST: " << cost;
#ifndef DEBUG
  cout << entry.str() << endl;
#endif
  write(entry.str());
}


/* general */
void Logger::debugInt(string entity, string debugString, int value) {
  stringstream entry; 
  entry << "[" << entity << "][" << currentTime << "] " << debugString << ": ";
  entry << value;
  write(entry.str());
}

void Logger::debugInt(string debugString, int value) {
  debugInt("DEBUG", debugString, value);
}

void Logger::debug(string entity, string debugString) {
  stringstream entry;
  entry << "[" << entity << "][" << currentTime << "] " << debugString;
  write(entry.str());
}

void Logger::debug(string debugString) {
  debug("DEBUG", debugString);
}

void Logger::info(string infoString) {
  // do stuff
}

void Logger::finalize() {
  logFile.close();
}
