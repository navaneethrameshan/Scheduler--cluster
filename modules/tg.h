#ifndef __TG__
#define __TG__

#include"task.h"
#include<iostream>
#include<time.h>
#include <list>
#include<cstdlib>
#include "Scheduler.h"
using namespace std;

class Taskgen{
	
	list<Task > list1;
	list<Job *> list2;	
	Scheduler *scheduler;

	public:
	Taskgen(Scheduler *sched);
	list<Task > create_task(Task *given_task, Job job[][100], int total_input);
	void add_task_list(Task *given_task);
	int add_job_list(Task *given_task, int rate, int start_pos);
	void send_task();
	void showtask();
	void showjob();
	};
#endif