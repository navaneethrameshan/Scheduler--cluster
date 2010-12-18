#ifndef __LOGGER__
#define __LOGGER__

#include <iostream>
#include <fstream>
using namespace std;

class Logger {
 private:
  ofstream logFile;
  void write(string entry);

  static bool loggerFlag;
  static Logger* loggerInstance;
  Logger();
  
 public:
  static Logger* getLogger();
  ~Logger() {
    loggerFlag = false;
  };
  void finalize();

  // logging functions
  void debug(string entity, string debugString);
  void debug(string debugString);
  void debugInt(string debugString, int value);
  void debugInt(string entity, string debugString, int value);
  void workerInt(string wString, int value);
  void workerFloat(string wString, float value);
  void info(string infoString);

  // scheduler
  void status(string mode, float interval, int queue, int running, int completed);
  void decision(string decision);

  // simulator
  void workerAverage(double avg_response_time, float cost,
                     int offline, int idle, int computing, int jobs);
  void totals(int job_count, long exec, long cpu, float cost, double job_avg_time, double deviation);
  
};

#endif /* __LOGGER__ */
