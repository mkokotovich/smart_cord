
#include <ESP8266WiFi.h>
#include "Timer.h"
#include "private.h"
#include "RestUI.h"
#include "OTAUpdates.h"

const char* ssid     = private_ssid;
const char* password = private_password;

// pin to control relay
int relay_pin = D5;

// Debug light, to indicate if the board has the relay pin activated
int debug_pin = D4;
int DEBUG = 1;

// Variable to hold the current state of relay, which will be displayed
String current_state = "UNINITIALIZED";
String onTimer_state = "Disabled";
String offTimer_state = "Disabled";

Timer onTimer = Timer();
Timer offTimer = Timer();

void controlRelay(int state)
{
  digitalWrite(relay_pin, state);
  if (DEBUG)
  {
    digitalWrite(debug_pin, state);
  }
}

void startWifi(void)
{
  // Connect to WiFi
  WiFi.disconnect(); //no-op if not connected
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("");
  Serial.println("Connecting to " + String(ssid));
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup(void)
{  
  // Start Serial
  Serial.begin(115200);

  // Turn pin off before setting it as output
  powerOff("");
  
  pinMode(relay_pin, OUTPUT);
  if (DEBUG)
  {
    pinMode(debug_pin, OUTPUT);
  }

  setupRestUI();
  
  startWifi();
  startRestUIServer();
  startUpdateServer();
}

void loop() {
  // Reset device if wifi is disconnected
  if (WiFi.status() == WL_DISCONNECTED)
  {
    Serial.println("Wifi diconnected, reset connection");
    
    startWifi();
    startRestUIServer();
    startUpdateServer();
  }

  // Handle onTimer
  if (onTimer.isActive())
  {
    if (onTimer.isExpired(onTimer_state))
    {
      Serial.println("OnTimer is up!");
      powerOn("");
    }
  }
  // Handle offTimer
  if (offTimer.isActive())
  {
    if (offTimer.isExpired(offTimer_state))
    {
      Serial.println("OffTimer is up!");
      powerOff("");
    }
  }

  handleOTAUpdate();
  handleRestUI();
 
}
