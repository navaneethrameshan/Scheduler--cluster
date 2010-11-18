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

void Logger::workerInt(string debugString, int value) {
  debugInt("WORKER", debugString, value);
}

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

void Logger::workerAverage(int offline, int idle, int computing, int jobs) {
  stringstream entry;
  entry << "-" << currentTime << "\t" 
        << offline << "\t"
        << idle << "\t"
        << computing << "\t"
        << jobs;
  write(entry.str());
}

void Logger::totals(long exec, long cpu, float cost) {
  stringstream entry;
  entry << "[SUMMARY] CPU: " << cpu
        << " EXECUTION: " << exec
        << " COST: " << cost;
  write(entry.str());
}

void Logger::finalize() {
  logFile.close();
}
