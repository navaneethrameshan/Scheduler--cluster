#include"tg.h"
#include "file.h"
#include <fstream>
using namespace std;

Taskgen::Taskgen(Scheduler *sched)
{
logger = Logger::getLogger();
scheduler = sched;

}
list<Task > Taskgen::create_task(Task  *given_task, Job job[][100], int total_input){
	 srand(time(NULL));
	 fstream file("modules/input.conf");
	 char type;
	 int sum=0;
	 for(int i=0; i<total_input; i++){
		file>> type;
		file>> TASK_ID;
		file>> NO_JOB;
		file>> RATE;
		file>> JOB_START_ID;
		file>>  LOW_INST_BOUND;
		file>> HIGH_INST_BOUND;
		file>> LOW_MEM;
		file>> HIGH_MEM;
		if (type == 's' || type == 'S'){ 
	  		for(int k=0;k<NO_JOB;k++){
	  			MEM= LOW_MEM+rand()%((HIGH_MEM-LOW_MEM!=0)?(HIGH_MEM-LOW_MEM):1);
	  			INST=LOW_INST_BOUND+rand()%((HIGH_INST_BOUND-LOW_INST_BOUND!=0)?(HIGH_INST_BOUND-LOW_INST_BOUND):1);
	         		job[i][k].init(TASK_ID,(k+JOB_START_ID),INST, MEM);
        		 	}
         		given_task[i].init(TASK_ID,NO_JOB,job[i],RATE); 
         		add_task_list(given_task+i);
         		}
         	else{
			for(int k=0;k<NO_JOB;k++){
	  			MEM= LOW_MEM+rand()%((HIGH_MEM-LOW_MEM!=0)?(HIGH_MEM-LOW_MEM):1);
	  			INST=LOW_INST_BOUND+rand()%((HIGH_INST_BOUND-LOW_INST_BOUND!=0)?(HIGH_INST_BOUND-LOW_INST_BOUND):1);
	         		job[k+sum][0].init(TASK_ID,(k+JOB_START_ID),INST, MEM);
				given_task[k+sum].init(TASK_ID,1,job[k+sum],RATE); 
				add_task_list(given_task+k+sum);
        		 	}
        		 sum = sum + NO_JOB;
        		 }
         	
         	}
         showtask();
         return (list1); 
 }
 
 void Taskgen::add_task_list(Task *given_task){
	 list1.push_back(*given_task);
 }
 
 int Taskgen::add_job_list(Task *given_task, int rate, int start_pos){
 	for(int i=0;i<rate;i++){
 		list2.push_back(given_task->jobs_list);
 		if(++start_pos<given_task->no_jobs)
 			given_task->jobs_list++;
 		else{
 			start_pos=0;
 			break;
 		}
 	}
 	showjob();
 	return start_pos;
 }
 
 void Taskgen:: send_task(){
	list<Job > templist;
	list<Job *>::iterator i;
        for(i=list2.begin(); i != list2.end(); ++i){
       		templist.push_back(*(*i));
       }
	scheduler->submitJobs(templist);
	list2.clear();
	templist.clear();

 }
 
 void Taskgen:: showtask(){
 	list<Task  >::iterator j;
       for(j=list1.begin(); j != list1.end(); ++j){
      		(*j).show();
      		cout<< endl;
       }	
 }
 
 void Taskgen:: showjob(){
 	list<Job * >::iterator j;
       for(j=list2.begin(); j != list2.end(); ++j){
      		logger->debug("Sending job: " + (*j)->show());
       }	
 }
