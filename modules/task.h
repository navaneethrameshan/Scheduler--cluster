#include "job.h"
class Task{
	
	int task_id;
	int job_rate;
	public:
	
	int no_jobs;
        Job *jobs_list;
       
       void init (int id, int jobs, Job *jobslist, int rate);
	
	void show();
		
	};