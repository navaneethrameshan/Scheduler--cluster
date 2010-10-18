//#include<iostream>
#include<stdio.h>
#include<cstdlib>
#include"modules/tg.h"
#include"modules/file.h"
using namespace std;

int counter=0;
int start_pos=0;

int main(){
 
 //Read File//
 	  Job job1[NO_JOBS];
         Task task1;
 Taskgen T;
 list<Task > tasklist;
  tasklist=T.create_task(&task1, job1); 
/* list<Task  >::iterator j;
      for(j=tasklist.begin(); j != tasklist.end(); j++){
      		(*j).show();
       }	*/
  // Universal Clock
  list<Task >::iterator i;
  i=tasklist.begin();
  while(1){
  	counter ++;
  	cout<<" Time ["<<counter<<"] ";
        start_pos=T.add_job_list(&(*i), RATE, start_pos);
	T.send_task();
	if (start_pos == 0)
		++i;
	if(i==tasklist.end())
		break;
  	}
  return 0;
}
