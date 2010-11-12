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
  if (DEBUG)
    cout << entry << endl;
}

/* public methods */
Logger* Logger::getLogger() {
  if (!loggerFlag) { 
    loggerInstance = new Logger;
    loggerFlag = true;
  }

  return loggerInstance;     
}

void Logger::debugInt(string debugString, int value) {
  stringstream entry; 
  entry << "[DEBUG][" << currentTime << "] " << debugString << ": ";
  entry << value;
  write(entry.str());
}

void Logger::debug(string debugString) {
  stringstream entry;
  entry << "[DEBUG][" << currentTime << "] " << debugString;
  write(entry.str());
}

void Logger::info(string infoString) {
  // do stuff
}

void Logger::workerAverage(int offline, int idle, int computing, int jobs) {
  stringstream entry;
  entry << "-" << currentTime << "\t" 
        << offline << "\t"
        << idle << "\t"
        << computing << "\t"
        << jobs;
  write(entry.str());
}

void Logger::totals(long exec, long cpu, long cost) {
  stringstream entry;
  entry << "[TOTALS] CPU: " << cpu
        << " EXECUTION: " << exec
        << " COST: " << cost;
  write(entry.str());
}

void Logger::finalize() {
  logFile.close();
}
