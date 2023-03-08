#pragma once
#include <MicroTasks.h>
#include <string>

class ThingsboardJob : public MicroTasks::Task {
public:
  ThingsboardJob();
  void setup();
  unsigned long loop(MicroTasks::WakeReason reason);
};