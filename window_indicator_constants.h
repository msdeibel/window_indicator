#ifndef window_indicator_constants.h
#define window_indicator_constants.h

const char* WI_SSID = "ssid";
const char* WI_PASSWORD = "password";
const long WI_WIFI_RETRY_INTERVALS_MS = 2000;
const int WI_WIFI_MAX_RETRIES = 5;

const int WI_AWAKE_WAKE_UP_AT_OCLOCK = 5;
const int WI_FALL_ASLEEP_AFTER_OCLOCK = 7;

const long WI_DEEPSLEEP_INTERVAL_US = 3e7; //30s 

const char* WI_NTP_SERVER_ADDRESS = "pool.ntp.org";
const int WI_NTP_SERVER_PORT = 1883;
const int WI_NTP_UPDATE_INTERVAL_MS = 600000; //10min
const int WI_NTP_TIMEZONE_OFFSET_S = 3600;

const int WI_BLINK_INTERVAL_MS = 400;

const char* WI_MQTT_SERVER = "mqttserver";
const char* WI_MQTT_CLIENT_NAME = "ESP8266";

#endif
