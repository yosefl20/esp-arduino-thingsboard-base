#pragma once
#include <MicroTasks.h>

class WifiJob : public MicroTasks::Task {
public:
  WifiJob();
  void setup();
  unsigned long loop(MicroTasks::WakeReason reason);
};