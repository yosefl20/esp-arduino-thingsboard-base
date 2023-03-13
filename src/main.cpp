#include <Arduino.h>
#include <MicroTasks.h>
#include <thingsboard_job.h>
#include <wifi_job.h>

WifiJob wifi;
ThingsboardJob tbjob;

void setup() {
  Serial.begin(9600);
  MicroTask.startTask(wifi);
  MicroTask.startTask(tbjob);
}

void loop() { MicroTask.update(); }