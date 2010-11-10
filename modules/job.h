#ifndef __JOB__
#define __JOB__



class Job{
	int task_id;	
	int no_inst;
	int mem_size;
	
	public:
	int job_id;
       
       void init(int taskid, int jobid, int inst, int mem);
       void show();
	//added by wasif
unsigned int getJobID();
// added by archie
 int getNumberOfInstructions();
	};


#endif /* __WORKER__ */
