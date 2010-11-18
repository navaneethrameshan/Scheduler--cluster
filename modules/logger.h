#ifndef __LOGGER__
#define __LOGGER__

#ifndef DEBUG
#define DEBUG true
#endif

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
  void info(string infoString);
  void workerAverage(int offline, int idle, int computing, int jobs);
  void totals(long exec, long cpu, float cost);
  
};

#endif /* __LOGGER__ */
