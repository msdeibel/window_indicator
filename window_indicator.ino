#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include ".\window_indicator_wifi_credentials.h"
#include ".\window_indicator_awake_times.h"

#define LED D0 //NodeMCU internal LED
#define LED1 D5
#define LED2 D6
#define LED3 D8

const long blinkIntervalInMs = 700;
const long ntpUpdateIntervalInMs = 6e5; //10min
const long deepSleepIntervalInus = 12e8; //20min


bool isWindowOpen = false;
int ledState = LOW;
unsigned long previousMsBlink = 0;
unsigned long previousMsNtp = 0;

WiFiServer server(80);
WiFiUDP ntpUdp;
NTPClient ntpClient(ntpUdp, "<NTP-Server IP or URI>", 3600);
 
void setup() {
  Serial.begin(115200);
  delay(2000);
  // The internal LED is automatically turned on in normal operation mode,
  // but not needed in this case, so turn it off right at the beginning.
  internalLedOff();

  setLedPinsModeToOutput();

  //Turn of any blinking leds
  toggleLeds();  

  // Connect to WiFi network
  ledState = HIGH;
  toggleLeds();
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  ledState = LOW;
  toggleLeds();

  // Start the NTP client
  ntpClient.begin();
  
  // Start the server
  server.begin();
}
 
void loop() {
  doNtpUpdate();

  if(ntpClient.getHours() < wakeUpAtHours || ntpClient.getHours() > fallAsleepAfterHours) {
    ESP.deepSleep(deepSleepIntervalIns);
  }
  
  doBlink();
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
 
  // Wait until the client sends some data
  while(!client.available()){
    delay(10);
  }
 
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  client.flush();

  handleRequest(request);

  sendHttp200ToClient(client);
}

void internalLedOff() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
}

void setLedPinsModeToOutput() {
  pinMode(LED1, OUTPUT);  
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
}

void doNtpUpdate(){
  unsigned long currentMs = millis();

  if(currentMs - previousMsNtp >= ntpUpdateIntervalInMs) {
    previousMsNtp = currentMs;
    ntpClient.update();
  }
}

void doBlink () {
  // This check is done using a check for the time passed to not prevent
  // the main loop from listening for new connections
  unsigned long currentMs = millis();
  
  if(isWindowOpen) {
    if(currentMs - previousMsBlink >= blinkIntervalInMs) {
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

void handleRequest(String request){
  if (request.indexOf("/LED=ON") != -1)  {
    isWindowOpen = true;
  } else if (request.indexOf("/LED=OFF") != -1) {
    isWindowOpen = false;
    ledState = LOW;
    toggleLeds();
  }
}

void sendHttp200ToClient(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println(""); //  do not forget this one
}

void toggleLeds() {
  digitalWrite(LED1, ledState);
  digitalWrite(LED2, ledState);
  digitalWrite(LED3, ledState);
}
