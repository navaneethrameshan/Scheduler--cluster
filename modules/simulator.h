#ifndef __SIMULATOR__
#define __SIMULATOR__

#include "worker.h"
#define DEBUG true

class Simulator {
  //Scheduler scheduler;
  //TaskGen *taskgenerator; 
  std::list<Worker *> workers;
  long clocktime;
  bool stopping;

 private:
  void tick();
  bool cleanUp();
  void debug(const char* msg);

 public:
  Simulator();
  void init();
  void execute();
  void stop();

};

#endif /* __SIMULATOR__ */
