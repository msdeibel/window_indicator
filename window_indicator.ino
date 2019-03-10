#include <ESP8266WiFi.h>
#include ".\window_indicator_wifi_credentials.h"

#define LED D0 //NodeMCU internal LED
#define LED1 D5
#define LED2 D6
#define LED3 D7

const long blinkIntervalInMs = 700;

bool isWindowOpen = false;
int ledState = LOW;
unsigned long previousMilliSeconds = 0;

WiFiServer server(80);
 
void setup() {
  Serial.begin(115200);
  delay(100);
  
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
  
  // Start the server
  server.begin();
}
 
void loop() {
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

  if (request.indexOf("/LED=ON") != -1)  {
    isWindowOpen = true;
  } else if (request.indexOf("/LED=OFF") != -1) {
    isWindowOpen = false;
    ledState = LOW;
    toggleLeds();
  }

  client.println("HTTP/1.1 200 OK");
  client.println(""); //  do not forget this one
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

void doBlink () {
  // This check is done using a check for the time passed to not prevent
  // the main loop from listening for new connections
  unsigned long currentMilliSeconds = millis();
  
  if(isWindowOpen) {
    if(currentMilliSeconds - previousMilliSeconds >= blinkIntervalInMs) {
      previousMilliSeconds = currentMilliSeconds;

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
