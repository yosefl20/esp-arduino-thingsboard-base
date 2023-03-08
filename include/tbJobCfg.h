#pragma once

#if THINGSBOARD_ENABLE_PROGMEM
constexpr char CURRENT_FIRMWARE_TITLE[] PROGMEM = "<product-name>";
constexpr char CURRENT_FIRMWARE_VERSION[] PROGMEM = "1.0.0";
#else
constexpr char CURRENT_FIRMWARE_TITLE[] = "<product-name>";
constexpr char CURRENT_FIRMWARE_VERSION[] = "1.0.0";
#endif

#if THINGSBOARD_ENABLE_PROGMEM
constexpr char PROVISION_DEVICE_KEY[] PROGMEM = "<key>";
constexpr char PROVISION_DEVICE_SECRET[] PROGMEM = "<secret>";
#else
constexpr char PROVISION_DEVICE_KEY[] = "<key>";
constexpr char PROVISION_DEVICE_SECRET[] = "<secret>";
#endif

#if THINGSBOARD_ENABLE_PROGMEM
constexpr char DEVICE_NAME[] PROGMEM = "";
#else
constexpr char DEVICE_NAME[] = "";
#endif

#if THINGSBOARD_ENABLE_PROGMEM
constexpr char CREDENTIALS_TYPE[] PROGMEM = "credentialsType";
constexpr char CREDENTIALS_VALUE[] PROGMEM = "credentialsValue";
constexpr char CLIENT_ID[] PROGMEM = "clientId";
constexpr char CLIENT_PASSWORD[] PROGMEM = "password";
constexpr char CLIENT_USERNAME[] PROGMEM = "userName";
#else
constexpr char CREDENTIALS_TYPE[] = "credentialsType";
constexpr char CREDENTIALS_VALUE[] = "credentialsValue";
constexpr char CLIENT_ID[] = "clientId";
constexpr char CLIENT_PASSWORD[] = "password";
constexpr char CLIENT_USERNAME[] = "userName";
#endif

// Firmware state send at the start of the firmware, to inform the cloud about
// the current firmware and that it was installed correctly, especially
// important when using OTA update, because the OTA update sends the last
// firmware state as UPDATING, meaning the device is restarting if the device
// restarted correctly and has the new given firmware title and version it
// should then send thoose to the cloud with the state UPDATED, to inform any
// end user that the device has successfully restarted and does actually contain
// the version it was flashed too
#if THINGSBOARD_ENABLE_PROGMEM
constexpr char FW_STATE_UPDATED[] PROGMEM = "UPDATED";
#else
constexpr char FW_STATE_UPDATED[] = "UPDATED";
#endif

// Maximum amount of retries we attempt to download each firmware chunck over
// MQTT
#if THINGSBOARD_ENABLE_PROGMEM
constexpr uint8_t FIRMWARE_FAILURE_RETRIES PROGMEM = 5U;
#else
constexpr uint8_t FIRMWARE_FAILURE_RETRIES = 5U;
#endif
// Size of each firmware chunck downloaded over MQTT,
// increased packet size, might increase download speed
#if THINGSBOARD_ENABLE_PROGMEM
constexpr uint16_t FIRMWARE_PACKET_SIZE PROGMEM = 4096U;
#else
constexpr uint16_t FIRMWARE_PACKET_SIZE = 4096U;
#endif

// Thingsboard we want to establish a connection too
#if THINGSBOARD_ENABLE_PROGMEM
constexpr char THINGSBOARD_SERVER[] PROGMEM = "<thingsboard-url>";
#else
constexpr char THINGSBOARD_SERVER[] = "<thingsboard-url>";
#endif

#if THINGSBOARD_ENABLE_PROGMEM
constexpr uint16_t THINGSBOARD_PORT PROGMEM = 1883U;
#else
constexpr uint16_t THINGSBOARD_PORT = 1883U;
#endif

// Maximum size packets will ever be sent or received by the underlying MQTT
// client, if the size is to small messages might not be sent or received
// messages will be discarded
#if THINGSBOARD_ENABLE_PROGMEM
constexpr uint32_t MAX_MESSAGE_SIZE PROGMEM = 512U;
#else
constexpr uint32_t MAX_MESSAGE_SIZE = 512U;
#endif
