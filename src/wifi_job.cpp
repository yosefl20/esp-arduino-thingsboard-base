#include "wifi_job.h"
#include <WiFiManager.h>

WiFiManager wm;

WifiJob::WifiJob() {}

void WifiJob::setup() {

  Serial.println("WifiJob::setup");
  WiFi.mode(WIFI_STA);
  wm.setConfigPortalBlocking(false);
  // wm.setConfigPortalTimeout(60);
  // automatically connect using saved credentials if they exist
  // If connection fails it starts an access point with the specified name
  if (wm.autoConnect()) {
    Serial.println("connected...yeey :)");
  } else {
    Serial.println("Configportal running");
  }
}

unsigned long WifiJob::loop(MicroTasks::WakeReason reason) {
  wm.process();
  return 10;
}
