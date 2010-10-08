#include"job.h"
#include<iostream>
using namespace std;

void job:: init(int id, int inst, int mem){
		job_id=id;
		no_inst=inst;
		mem_size=mem;
		}
		
void job:: show(){
	cout<< "\nJob ID: "<< job_id<<" Job Inst: "<<no_inst<<" Job Mem: "<<mem_size;
	}
		