#include "job.h"
class Task{
	
	int task_id;
	int no_jobs;
	job *jobs_list;
	
	public:
       
       void init (int id, int jobs, job *jobslist);
	
	void show();
		
	};