#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include ".\window_indicator_constants.h"

#define LED D0 //NodeMCU internal LED
#define LED1 D5
#define LED2 D6
#define LED3 D8

bool isWindowOpen = false;
int ledState = LOW;
unsigned long previousMsBlink = 0;
unsigned long previousMsNtp = 0;
int wifiRetryCounter = 0;

WiFiServer server(80);
WiFiUDP ntpUdp;
NTPClient ntpClient(ntpUdp, WI_NTP_SERVER_ADDRESS, 3600);
 
void setup() {
  // The internal LED is automatically turned on in normal operation mode,
  // but not needed in this case, so turn it off right at the beginning.
  internalLedOff();

  setLedPinsModeToOutput();

  //Turn of any blinking leds
  toggleLeds();  

  // Connect to WiFi network
  WiFi.begin(WI_SSID, WI_PASSWORD);
 
  while (WiFi.status() != WL_CONNECTED) {
    if(wifiRetryCounter++ < WI_WIFI_MAX_RETRIES) {     
      delay(WI_WIFI_RETRY_INTERVALS_MS);
    } else {

      digitalWrite(LED1, HIGH); delay(100); digitalWrite(LED1, LOW); delay(100); 
      digitalWrite(LED1, HIGH); delay(100); digitalWrite(LED1, LOW); delay(100); 
      digitalWrite(LED1, HIGH); delay(100); digitalWrite(LED1, LOW); delay(100); 
      digitalWrite(LED1, HIGH); delay(100); digitalWrite(LED1, LOW); delay(100); 
      digitalWrite(LED1, HIGH); delay(100); digitalWrite(LED1, LOW); 
      
      ESP.deepSleep(WI_DEEPSLEEP_INTERVAL_US);
    }
  }

  // Start the NTP client
  ntpClient.begin();
  
  // Start the server
  server.begin();
}
 
void loop() {
  doNtpUpdate();

  if(ntpClient.getHours() < WI_AWAKE_WAKE_UP_AT_OCLOCK 
      || ntpClient.getHours() > WI_FALL_ASLEEP_AFTER_OCLOCK) {
    ESP.deepSleep(WI_DEEPSLEEP_INTERVAL_US);
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

  if(currentMs - previousMsNtp >= WI_NTP_UPDATE_INTERVAL_MS) {
    previousMsNtp = currentMs;
    ntpClient.update();
  }
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
