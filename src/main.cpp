#include <Arduino.h>
#include <MicroTasks.h>
#include <tbJob.h>
#include <wifiJob.h>

WifiJob wifi;
ThingsboardJob tbjob;

void setup() {
  Serial.begin(9600);
  MicroTask.startTask(wifi);
  MicroTask.startTask(tbjob);
}

void loop() { MicroTask.update(); }