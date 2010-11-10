#include <iostream>
#include <fstream>

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
  string entry = "[DEBUG] " + debugString;
  write(entry);
}

void Logger::info(string infoString) {
  // do stuff
}

void Logger::finalize() {
  logFile.close();
}
