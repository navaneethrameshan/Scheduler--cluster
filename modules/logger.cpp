#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "logger.h"

using namespace std;

bool Logger::loggerFlag = false;
Logger* Logger::loggerInstance = NULL;

/* private methods */
Logger::Logger() {
  logFile.open("cloud.log");
}

void Logger::write(string entry) {
  logFile << entry << endl;
}

/* public methods */
Logger* Logger::getLogger() {
  if (!loggerFlag) { 
    loggerInstance = new Logger;
    loggerFlag = true;
  }

  return loggerInstance;     
}

void Logger::debug(string debugString) {
  stringstream entry;
  entry << "[DEBUG] " << debugString;
  write(entry.str());
}

void Logger::info(string infoString) {
  // do stuff
}

void Logger::workerAverage(long time, int workers, int jobs) {
  stringstream entry;
  entry << "-" << time << "\t" << workers << "\t" << jobs;
  write(entry.str());
}

void Logger::finalize() {
  logFile.close();
}
