#include"task.h"
#include<iostream>
using namespace std;

void Task:: init(int id, int jobs, Job *jobslist, int rate ){
		task_id=id;
		no_jobs=jobs;
		jobs_list = jobslist;	
		job_rate=rate;
		}
		
 void Task:: show(){
		cout<<" \nTASK::  Task ID: "<<task_id<<" No. of Jobs: "<<no_jobs<< " Rate: "<<job_rate;
		Job *temp=jobs_list;
		for(int i=0;i<no_jobs;i++){
			temp->show();
			temp++;
			}
		}