#include"task.h"
#include<iostream>
using namespace std;

void Task:: init(int id, int jobs, job *jobslist ){
		task_id=id;
		no_jobs=jobs;
		jobs_list = jobslist;	
		}
		
 void Task:: show(){
		cout<<"Task ID: "<<task_id<<" No. of Jobs: "<<no_jobs;
		for(int i=0;i<no_jobs;i++){
			jobs_list->show();
			jobs_list++;
			}
		}