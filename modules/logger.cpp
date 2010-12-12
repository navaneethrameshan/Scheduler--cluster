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

void Logger::workerFloat(string debugString, float value) {
  stringstream s; 
  s << "[WORKER]" << "[" << currentTime << "]" << debugString << ": " << value;
  write(s.str());
}


/* for simulator */ 
void Logger::workerAverage(double avg_response_time, float cost,
                           int offline, int idle, int computing, int jobs) {
  int active = idle + computing;

  stringstream entry;
  entry << "-" << currentTime/1000 << "\t" // in s 
        << avg_response_time << "\t"
        << cost << "\t"
        << active << "\t"
        << offline << "\t"
        << idle << "\t"
        << computing << "\t"
        << jobs;
  write(entry.str());
}

void Logger::totals(int job_count, long exec, long cpu, float cost, double avgtime) {
  stringstream entry;
  entry << "---------------- [SIMULATOR] ----------------- \n" 
        << "Number of jobs:\t\t" << job_count << "\n"
        << "Total:\t\t\t" << cpu << "s\n"
        << "Active:\t\t\t" << exec << "s\n"
        << "Unused:\t\t\t" << cpu-exec << "s\n"
        << "Cost: \t\t\t" << cost << " Euro\n"
        << "Job avg response time: \t" << avgtime << "s\n"
        << "Standard deviation: \tXXs\n" 
        << "----------------------------------------------";
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
