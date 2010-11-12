#ifndef __SIMULATOR__
#define __SIMULATOR__

#include "worker.h"
#include "Scheduler.h"
/*copied from Navaneeths main*/
#include<stdio.h>
#include<cstdlib>
#include"tg.h"
#include "logger.h"

#ifndef DEBUG
#define DEBUG false
#endif

extern long currentTime;

class Simulator {  
 private:
  Logger* logger;
  list<Task > tasklist;
  list<Worker *> workers;
  bool stopping;

  bool cleanUp();
  void debug(const char* msg);
  int start_pos;
  bool readWorkers(Scheduler* scheduler);
  void runWorkers();
  void logRunningAverage();
  

 public:
  Simulator();
  void execute();
  void stop();

};

#endif /* __SIMULATOR__ */
