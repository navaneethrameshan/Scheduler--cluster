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
  void debug(string debugString);
  void debugInt(string debugString, int value);
  void info(string infoString);
  void workerAverage(long time, int workers, int jobs);
  void totals(long exec, long cpu, long cost);
  
};

#endif /* __LOGGER__ */
