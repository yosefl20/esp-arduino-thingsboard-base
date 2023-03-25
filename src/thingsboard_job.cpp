#include "thingsboard_job.h"
#include "thingsboard_job_cfg.h"
#include <EEPROM.h>
#include <ThingsBoard.h>
#include <thingsboard_messages.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
// Disable PROGMEM because the ESP8266WiFi library,
// does not support flash strings.
#define THINGSBOARD_ENABLE_PROGMEM 0
#elif defined(ESP32)
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif

WiFiClient espClient;
ThingsBoard tb(espClient, MAX_MESSAGE_SIZE);

// Statuses for updating
bool currentFWSent = false;
bool updateRequestSent = false;

uint32_t previous_processing_time = 0U;
// Statuses for provisioning
bool provisionRequestSent = false;
bool provisionResponseProcessed = false;

// Struct for client connecting after provisioning
struct Credentials {
  std::string client_id;
  std::string username;
  std::string password;
};
Credentials credentials;

/// @brief Updated callback that will be called as soon as the firmware update
/// finishes
/// @param success Either true (update succesfull) or false (update failed)
void updatedCallback(const bool &success) {
  if (success) {
#if THINGSBOARD_ENABLE_PROGMEM
    Serial.println(F("Done, Reboot now"));
#else
    Serial.println("Done, Reboot now");
#endif
#if defined(ESP8266)
    ESP.restart();
#elif defined(ESP32)
    esp_restart();
#endif
    return;
  }
#if THINGSBOARD_ENABLE_PROGMEM
  Serial.println(F("Downloading firmware failed"));
#else
  Serial.println("Downloading firmware failed");
#endif
}

/// @brief Progress callback that will be called every time we downloaded a new
/// chunk successfully
/// @param currentChunk
/// @param totalChuncks
void progressCallback(const uint32_t &currentChunk,
                      const uint32_t &totalChuncks) {
  Serial.printf("Progress %.2f%%\n",
                static_cast<float>(currentChunk * 100U) / totalChuncks);
}

#if defined(ESP8266)

int writeFlag(char add) {
  EEPROM.write(add, 'T');
  EEPROM.write(add + 1, 'B');
  EEPROM.commit();
  return 2;
}

int checkFlag(char add) {
  char flag[3];
  flag[0] = EEPROM.read(add);
  flag[1] = EEPROM.read(add + 1);
  flag[2] = '\0';
  return strcmp(flag, "TB") == 0 ? 2 : 0;
}

int writeString(char add, const std::string &data) {
  int _size = data.length();
  int i;
  for (i = 0; i < _size; i++) {
    EEPROM.write(add + i, data[i]);
  }
  EEPROM.write(add + _size,
               '\0'); // Add termination null character for String Data
  EEPROM.commit();
  return _size + 1;
}

int read_String(char add, std::string &str) {
  char data[100]; // Max 100 Bytes
  int len = 0;
  unsigned char k;
  k = EEPROM.read(add);
  while (k != '\0' && len < 99) // Read until null character
  {
    k = EEPROM.read(add + len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';
  str.assign(data, len);
  return len + 1;
}
#endif

void loadCfg() {
#if defined(ESP8266)
  EEPROM.begin(512);
  int addr = 0;
  addr += checkFlag(addr);
  if (addr == 0) {
    Serial.println("check flag failed, abort load cfg");
    return;
  }
  Serial.println("read client id");
  addr += read_String(addr, credentials.client_id);
  Serial.println("read user name");
  addr += read_String(addr, credentials.username);
  Serial.println("read password");
  addr += read_String(addr, credentials.password);
#elif defined(ESP32)
  Preferences prefs;
  prefs.begin("thingsboard");
  if (prefs.isKey("cid")) {
    credentials.client_id = prefs.getString("cid", "").c_str();
  }
  if (prefs.isKey("username")) {
    credentials.username = prefs.getString("username", "").c_str();
  }
  if (prefs.isKey("password")) {
    credentials.password = prefs.getString("password", "").c_str();
  }
  prefs.end();
#endif
}

void saveCfg() {
#if defined(ESP8266)
  int addr = 0;
  addr += writeFlag(addr);
  addr += writeString(addr, credentials.client_id);
  addr += writeString(addr, credentials.username);
  addr += writeString(addr, credentials.password);
#elif defined(ESP32)
  Preferences prefs;
  prefs.begin("thingsboard");
  Serial.println(prefs.freeEntries());
  prefs.putString("cid", credentials.client_id.c_str());
  prefs.putString("username", credentials.username.c_str());
  prefs.putString("password", credentials.password.c_str());
  prefs.end();
#endif
}

void clearCfg() {
#if defined(ESP8266)
  for (int i = 0; i < 512; ++i)
    EEPROM.write(i, 0);
  EEPROM.commit();
#elif defined(ESP32)
  Preferences prefs;
  prefs.begin("thingsboard");
  prefs.remove("cid");
  prefs.remove("username");
  prefs.remove("password");
  prefs.end();
#endif
}

void processProvisionResponse(const Provision_Data &data) {
  int jsonSize = JSON_STRING_SIZE(measureJson(data));
  char buffer[jsonSize];
  serializeJson(data, buffer, jsonSize);
  Serial.printf("Received device provision response (%s)\n", buffer);

  if (strncmp(data["status"], "SUCCESS", strlen("SUCCESS")) != 0) {
    Serial.printf("Provision response contains the error: (%s)\n",
                  data["errorMsg"].as<const char *>());
    return;
  }

  if (strncmp(data[CREDENTIALS_TYPE], ACCESS_TOKEN_CRED_TYPE,
              strlen(ACCESS_TOKEN_CRED_TYPE)) == 0) {
    credentials.client_id = "";
    credentials.username = data[CREDENTIALS_VALUE].as<std::string>();
    credentials.password = "";
    saveCfg();
  } else if (strncmp(data[CREDENTIALS_TYPE], MQTT_BASIC_CRED_TYPE,
                     strlen(MQTT_BASIC_CRED_TYPE)) == 0) {
    auto credentials_value = data[CREDENTIALS_VALUE].as<JsonObjectConst>();
    credentials.client_id = credentials_value[CLIENT_ID].as<std::string>();
    credentials.username = credentials_value[CLIENT_USERNAME].as<std::string>();
    credentials.password = credentials_value[CLIENT_PASSWORD].as<std::string>();
    saveCfg();
  } else {
    Serial.printf("Unexpected provision credentialsType: (%s)\n",
                  data[CREDENTIALS_TYPE].as<const char *>());
    return;
  }

  // Disconnect from the cloud client connected to the provision account,
  // because it is no longer needed the device has been provisioned and we can
  // reconnect to the cloud with the newly generated credentials.
  if (tb.connected()) {
    tb.disconnect();
  }
  provisionResponseProcessed = true;
}

const OTA_Update_Callback callback(&progressCallback, &updatedCallback,
                                   CURRENT_FIRMWARE_TITLE,
                                   CURRENT_FIRMWARE_VERSION,
                                   FIRMWARE_FAILURE_RETRIES,
                                   FIRMWARE_PACKET_SIZE);

ThingsboardJob::ThingsboardJob() {}

void ThingsboardJob::setup() {
  Serial.println("ThingsboardJob::setup");
  loadCfg();
  previous_processing_time = millis();
  provisionRequestSent = provisionResponseProcessed =
      credentials.client_id.length() > 0 || credentials.username.length() > 0 ||
      credentials.password.length() > 0;
}

unsigned long ThingsboardJob::loop(MicroTasks::WakeReason reason) {

  if (WakeReason_Message == reason) {
    MicroTasks::Message *msg;
    if (this->receive(msg)) {
      if (tb.connected()) {
        // Serial.print("Got message ");
        // Serial.println(TBTelemetryMessage<float, 1000>::ID() == msg->id()
        //                    ? "TBTelemetryMessage"
        //                    : "UNKNOWN");
        // if (TBTelemetryMessage<float, 1000>::ID() == msg->id()) {
        //   TBTelemetryMessage<float, 1000> *telemetry =
        //       static_cast<TBTelemetryMessage<float, 1000> *>(msg);
        //   telemetry->sendIot(tb);
        // }
      }
      delete msg;
    }
    return 10 | MicroTask.WaitForMessage;
  }

  if (WiFi.status() == WL_NO_SHIELD) {
#if THINGSBOARD_ENABLE_PROGMEM
    Serial.println(F("WiFi shield not present"));
#else
    Serial.println("WiFi shield not present");
#endif
    return 1000 | MicroTask.WaitForMessage;
  }
  if (millis() - previous_processing_time < 1000) {
    return 500 | MicroTask.WaitForMessage;
  }
  previous_processing_time = millis();

  if (!provisionRequestSent) {
    if (!tb.connected()) {
      // Connect to the ThingsBoard server as a client wanting to provision a
      // new device
      Serial.printf("Connecting to: (%s)\n", THINGSBOARD_SERVER);
      if (!tb.connect(THINGSBOARD_SERVER, "provision", THINGSBOARD_PORT)) {
#if THINGSBOARD_ENABLE_PROGMEM
        Serial.println(F("Failed to connect"));
#else
        Serial.println("Failed to connect");
#endif
        return 100 | MicroTask.WaitForMessage;
      }
#if THINGSBOARD_ENABLE_PROGMEM
      Serial.println(F("Sending provisioning request"));
#else
      Serial.println("Sending provisioning request");
#endif
#if USE_MAC_FALLBACK
      // Check if passed DEVICE_NAME was empty,
      // and if it was get the mac address of the wifi chip as fallback and use
      // that one instead
      const Provision_Callback provisionCallback(
          Access_Token(), &processProvisionResponse, PROVISION_DEVICE_KEY,
          PROVISION_DEVICE_SECRET,
          (DEVICE_NAME != NULL) && (DEVICE_NAME[0] == '\0')
              ? DEVICE_NAME
              : WiFi.macAddress().c_str());
#else
      // Send a claiming request without any device name (access token will be
      // used as the device name) if the string is empty or null, automatically
      // checked by the sendProvisionRequest method
      const Provision_Callback provisionCallback(
          Access_Token(), &processProvisionResponse, PROVISION_DEVICE_KEY,
          PROVISION_DEVICE_SECRET, DEVICE_NAME);
#endif
      provisionRequestSent = tb.Provision_Request(provisionCallback);
    }
  } else if (provisionResponseProcessed) {
    if (!tb.connected()) {
      // Connect to the ThingsBoard server, as the provisioned client
      Serial.printf("Connecting to: (%s)\n", THINGSBOARD_SERVER);
      if (!tb.connect(THINGSBOARD_SERVER, credentials.username.c_str(),
                      THINGSBOARD_PORT, credentials.client_id.c_str(),
                      credentials.password.c_str())) {
#if THINGSBOARD_ENABLE_PROGMEM
        Serial.println(F("Failed to connect"));
#else
        Serial.println("Failed to connect");
#endif
        Serial.println(credentials.client_id.c_str());
        Serial.println(credentials.username.c_str());
        Serial.println(credentials.password.c_str());
        // wipped out eeprom
        clearCfg();
        provisionRequestSent = provisionResponseProcessed = false;
        return 1000 | MicroTask.WaitForMessage;
      } else {
#if THINGSBOARD_ENABLE_PROGMEM
        Serial.println(F("Connected!"));
#else
        Serial.println("Connected!");
#endif
      }
    } else {

      if (!currentFWSent) {
        currentFWSent = tb.Firmware_Send_Info(CURRENT_FIRMWARE_TITLE,
                                              CURRENT_FIRMWARE_VERSION) &&
                        tb.Firmware_Send_State(FW_STATE_UPDATED);
      }

      if (!updateRequestSent) {
#if THINGSBOARD_ENABLE_PROGMEM
        Serial.println(F("Firwmare Update..."));
#else
        Serial.println("Firwmare Update...");
#endif
        // See https://thingsboard.io/docs/user-guide/ota-updates/
        // to understand how to create a new OTA pacakge and assign it to a
        // device so it can download it.
        updateRequestSent = tb.Start_Firmware_Update(callback);
      }

      // send data here
      // #if THINGSBOARD_ENABLE_PROGMEM
      //       Serial.println(F("Sending telemetry..."));
      // #else
      //       Serial.println("Sending telemetry...");
      // #endif
      //       tb.sendTelemetryInt(TEMPERATURE_KEY, 22);
      //       tb.sendTelemetryFloat(HUMIDITY_KEY, 42.5);
    }
  }

  tb.loop();
  return 10 | MicroTask.WaitForMessage;
}
