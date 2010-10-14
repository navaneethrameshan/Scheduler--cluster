#ifndef __WORKER__
#define __WORKER__

enum states { INITIALISING, COMPUTING, FINALISING };

struct WORKER_STATE {
  enum states current;
  int tick;
};

class Worker {
  int id;
  WORKER_STATE state;

 public:
  Worker(int id);
  void execute();

 private:
  void initialise();
  void compute();
  void finalise();
  bool setState(enum states newstate);
  void tick();
};

#endif /* __WORKER__ */
