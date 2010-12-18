#ifndef __SIMULATOR__
#define __SIMULATOR__

#include "worker.h"
#include "Scheduler.h"
/*copied from Navaneeths main*/
#include<stdio.h>
#include<cstdlib>
#include"tg.h"
#include "logger.h"

extern unsigned long currentTime;

struct SIMULATOR_CONFIG {
  string scheduler_mode;
  float scheduling_interval;
  unsigned short scheduling_interval_for_clock;
  float worker_node_speed;
  float worker_node_memory;
  float worker_node_swapping_cost;
  float worker_quantum;
  float worker_node_startup_time;
  float worker_node_sched_notif_time;
  float worker_node_cost;
  int sliding_window; // seconds
  int polling_interval; // seconds
  int percentage_waste;
};

class Simulator {  
 private:
  Logger* logger;
  list<Task > tasklist;
  list<Worker *> workers;
  list<double> slidingWindow;
  bool stopping;
  SIMULATOR_CONFIG* config;

  bool cleanUp();
  int start_pos;
  bool readWorkers(Scheduler* scheduler);
  void runWorkers();
  void logRunningAverage();
  void logTotals();
  SIMULATOR_CONFIG* readSimulatorConfig();
  double getWorkerAverages();

 public:
  Simulator();
  void execute();
  void stop();

};

#endif /* __SIMULATOR__ */
