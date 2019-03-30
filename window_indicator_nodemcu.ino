#include <ESP8266WiFi.h>
#include <EasyNTPClient.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include ".\window_indicator_constants.h"

#define LED1 14
#define LED2 12
#define LED3 15

bool isWindowOpen = false;
bool isWindowBatteryLow = false;
int ledState = LOW;
unsigned long previousMsBlink = 0;
unsigned long previousMsNtp = 0;
int wifiRetryCounter = 0;
int hourOfDay = 0;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
WiFiUDP ntpUdp;
EasyNTPClient ntpClient(ntpUdp, WI_NTP_SERVER_ADDRESS, WI_NTP_TIMEZONE_OFFSET_S);
 
void setup() {
  setLedPinsModeToOutput();

  //Turn of any blinking leds
  toggleLeds();  

  WiFi.begin(WI_SSID, WI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    if(wifiRetryCounter++ < WI_WIFI_MAX_RETRIES) {     
      delay(WI_WIFI_RETRY_INTERVALS_MS);
    } else {
      ESP.deepSleep(WI_DEEPSLEEP_INTERVAL_US);
    }
  }

  mqttClient.setServer(WI_MQTT_SERVER, WI_NTP_SERVER_PORT);
  mqttClient.setCallback(mqttCallback);

  hourOfDay = (ntpClient.getUnixTime()/3600)%24;
}
 
void loop() {
  doNtpUpdate();
    
  if(hourOfDay < WI_AWAKE_WAKE_UP_AT_OCLOCK 
      || hourOfDay > WI_FALL_ASLEEP_AFTER_OCLOCK) {
    ESP.deepSleep(WI_DEEPSLEEP_INTERVAL_US);
  }
  
  if(!mqttClient.connected()) {
    reconnect();
  }

  mqttClient.loop();  

  doBlink();

  if(isWindowOpen){
    delay(100);  
  }
  else{
    ESP.deepSleep(WI_DEEPSLEEP_INTERVAL_US);
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char message[length+1];
  String messageString;
  
  if(strcmp(topic, "badfenster") == 0) {

    for (int i = 0; i < length; i++) {
      message[i] = (char)payload[i];
    }

    message[length] = '\0';
    messageString = String(message);
    
    handleRequest(message);
  }
}



void doNtpUpdate(){
  unsigned long currentMs = millis();

  if(currentMs - previousMsNtp >= WI_NTP_UPDATE_INTERVAL_MS) {
    previousMsNtp = currentMs;
    hourOfDay = (ntpClient.getUnixTime()/3600)%24;
  }
}

boolean reconnect() {
  if (mqttClient.connect(WI_MQTT_CLIENT_NAME)) {
    mqttClient.subscribe("badfenster");
  }  
  
  return mqttClient.connected();
}

void handleRequest(String request){
  if (request.indexOf("open") != -1)  {
    isWindowOpen = true;
  } else if (request.indexOf("closed") != -1) {
    isWindowOpen = false;
    ledState = LOW;
    toggleLeds();
  } 
}



void setLedPinsModeToOutput() {
  pinMode(LED1, OUTPUT);  
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
}

void doBlink () {
  // This check is done using a check for the time passed to not prevent
  // the main loop from listening for new connections
  unsigned long currentMs = millis();
  
  if(isWindowOpen) {
    if(currentMs - previousMsBlink >= WI_BLINK_INTERVAL_MS) {
      previousMsBlink = currentMs;

      if(ledState == LOW){
        ledState = HIGH; 
      } else {
        ledState = LOW;
      }

      toggleLeds();
    }
  } 
}

void toggleLeds() {
  digitalWrite(LED1, ledState);
  digitalWrite(LED2, ledState);
  digitalWrite(LED3, ledState);
}
