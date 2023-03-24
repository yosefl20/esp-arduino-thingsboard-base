#pragma once

#include "MicroTasks.h"
#include "MicroTasksMessage.h"
#include <string>

template <typename T, uint32_t tID>
class TBTelemetryMessage : public MicroTasks::Message {
private:
  std::string m_name;
  T m_value;

public:
  TBTelemetryMessage(const std::string &name, T value)
      : MicroTasks::Message(tID), m_name(name), m_value(value) {}

  static const uint32_t ID() { return tID; }
  inline bool sendIot(ThingsBoard &tb) {
    return tb.sendTelemetryData<T>(m_name.c_str(), m_value);
  }
};
