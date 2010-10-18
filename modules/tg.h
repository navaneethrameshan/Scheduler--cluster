#include"task.h"
#include"file.h"
#include<iostream>
#include<time.h>
#include <list>
using namespace std;

class Taskgen{
	
	list<Task > list1;
	list<Job *> list2;	
	public:
	list<Task > create_task(Task *given_task, Job *job);
	void add_task_list(Task *given_task);
	int add_job_list(Task *given_task, int rate, int start_pos);
	void send_task();
	void showtask();
	void showjob();
	};