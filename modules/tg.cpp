#include"tg.h"


Taskgen::Taskgen(Scheduler *sched)
{
scheduler = sched;

}
list<Task > Taskgen::create_task(Task * given_task, Job *job){
	 srand(time(NULL));
	  for(int i=0;i<NO_JOBS;i++){
         job[i].init(TASK_ID,(i+JOB_START_ID),INST, MEM);
         }
         given_task->init(TASK_ID,NO_JOBS,job,RATE);
         add_task_list(given_task);
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
       }	
 }
 
 void Taskgen:: showjob(){
 	list<Job * >::iterator j;
       for(j=list2.begin(); j != list2.end(); ++j){
      		(*j)->show();
       }	
 }
