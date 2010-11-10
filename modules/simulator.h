#ifndef __SIMULATOR__
#define __SIMULATOR__

#include "worker.h"
#include "Scheduler.h"
/*copied from Navaneeths main*/
#include<stdio.h>
#include<cstdlib>
#include"tg.h"
#include"file.h"

#define DEBUG true

extern long currentTime;

class Simulator {

  list<Task > tasklist;
  list<Worker *> workers;
  bool stopping;
 
 private:
  bool cleanUp();
  void debug(const char* msg);
  int start_pos;
  bool readWorkers(Scheduler* scheduler);
  void runWorkers();
  

 public:
  Simulator();
  void execute();
  void stop();

};

#endif /* __SIMULATOR__ */
