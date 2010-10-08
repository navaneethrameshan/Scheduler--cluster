#include"task.h"

class taskgen{
	Task *current_task;
	
	public:
	
	void store_task(Task *given_task);
	void send_task();
	};