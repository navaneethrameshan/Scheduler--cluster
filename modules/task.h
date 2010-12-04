#include "job.h"
class Task{
	
	int task_id;
	public:
	int job_rate;
	int no_jobs;
        Job *jobs_list;
       
       void init (int id, int jobs, Job *jobslist, int rate);
	
	void show();
		
	};