#include <queue>
#include "process.h"
#include <algorithm>
#include <map>
#include <thread>

class Scheduler
{
public:
  Scheduler(std::vector<Process*>& processes);
  void operator()();

//use maxheap or minheap
private:
  std::priority_queue<Process*> q1;
  std::priority_queue<Process*> q2;
  std::map<std::string, int> priorityUpdator;
  std::map<std::string, std::thread*> processThreads;
};
