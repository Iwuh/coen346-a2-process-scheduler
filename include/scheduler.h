#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include <map>
#include <thread>

#include "process.h"
#include "prioritycompare.h"

class Scheduler
{
public:
  Scheduler(std::vector<Process *> &processes);
  void operator()();

private:
  std::priority_queue<Process *, std::vector<Process *>, PriorityCompare> q1;
  std::priority_queue<Process *, std::vector<Process *>, PriorityCompare> q2;
  std::map<std::string, int> priorityUpdator;
  std::map<std::string, std::thread *> processThreads;
};

#endif