#ifndef __SIMULATOR__
#define __SIMULATOR__

#include "worker.h"

#define DEBUG true

extern long currentTime;

class Simulator {
  //Scheduler scheduler;
  //TaskGen *taskgenerator; 
  std::list<Worker *> workers;
  bool stopping;

 private:
  bool cleanUp();
  void debug(const char* msg);

 public:
  Simulator();
  void init();
  void execute();
  void stop();

};

#endif /* __SIMULATOR__ */
