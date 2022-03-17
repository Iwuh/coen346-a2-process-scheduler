#include <queue>
#include "include/process.h"
#include <algorithm>

class Scheduler
{
public:
  void operator()();

//use maxheap or minheap
private:
  std::priority_queue<Process*> q1;
  std::priority_queue<Process*> q2;
  std::map<std::string, int> priorityUpdator;
};
