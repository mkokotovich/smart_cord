
#include <ESP8266WiFi.h>
// Needs to be from https://github.com/mkokotovich/aREST until pull request is merged in
#include <aREST.h>
// Needs to be from https://github.com/mkokotovich/aREST_UI until pull request is merged in
#include <aREST_UI.h>
#include "Timer.h"
#include "private.h"

const char* ssid     = private_ssid;
const char* password = private_password;

#define READ_TIMEOUT 10000 //10 seconds

// pin to control relay
int relay_pin = D5;

// Debug light, to indicate if the board has the relay pin activated
int debug_pin = D4;
int DEBUG = 1;

// Create aREST instance
aREST_UI rest = aREST_UI();

// The port to listen for incoming TCP connections 
#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

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

int powerOn(String command) {
  controlRelay(LOW);
  current_state = "On";
  return 1;
}

int powerOff(String command) {
  controlRelay(HIGH);
  current_state = "Off";
  return 1;
}

int onTimerFunc(String command) {
  Serial.print("onTimerFunc called with: ");
  Serial.println(command);
  
  // Passed to us in minutes, need to convert
  unsigned long timer_ms_duration = strtoul(command.c_str(), NULL, 10) * 60 * 1000;
  onTimer.set(timer_ms_duration, onTimer_state);
  
  return 1;
}

int offTimerFunc(String command) {
  Serial.print("offTimerFunc called with: ");
  Serial.println(command);
  
  // Passed to us in minutes, need to convert
  unsigned long timer_ms_duration = strtoul(command.c_str(), NULL, 10) * 60 * 1000;
  offTimer.set(timer_ms_duration, offTimer_state);
  
  return 1;
}

int cancelOnTimer(String command) {
  onTimer.cancel(onTimer_state);
  return 1;
}

int cancelOffTimer(String command) {
  offTimer.cancel(offTimer_state);
  return 1;
}

void startWifi(void)
{
  // Connect to WiFi
  WiFi.disconnect(); //no-op if not connected
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void startWifiServer(void)
{
  // Start the server
  server.begin();
  Serial.println("Server started");
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

  // Display the current state
  rest.variable_label("current_state", "Current state of smart cord", &current_state);
  
  // Register the functions with the rest UI
  
  rest.function_button("powerOn", "Power On", powerOn);
  rest.function_button("powerOff", "Power Off", powerOff);
  
  rest.label("Turn on after:");
  rest.function_with_input_button("onTimerFunc", "minutes", onTimerFunc);
  // Display the timer state
  rest.variable_label("onTimer_state", "Minutes until timer turns the cord on", &onTimer_state);
  rest.function_button("cancelOffTimer", "Cancel Timer", cancelOnTimer);
  
  rest.label("Turn off after:");
  rest.function_with_input_button("offTimerFunc", "minutes", offTimerFunc);
  // Display the timer state
  rest.variable_label("offTimer_state", "Minutes until timer turns the cord off", &offTimer_state);
  rest.function_button("cancelOffTimer", "Cancel Timer", cancelOffTimer);
  
  // Give name and ID to device
  rest.set_id("1");
  rest.set_name("esp8266");
  rest.title("Smart Cord - coffee pot");
  
  startWifi();
  
  startWifiServer();
  
  // Print the IP address
  Serial.println(WiFi.localIP());
  
}

void loop() {
  // Reset device if wifi is disconnected
  if (WiFi.status() == WL_DISCONNECTED)
  {
    Serial.println("Wifi diconnected, reset connection");
    
    startWifi();
    startWifiServer();
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
  
  // Handle REST calls
  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }
  delay(50); //devices seems to choke if we read too fast, even with below wait
  unsigned long startMillis = millis();
  while (!client.available() && (millis() - startMillis) < READ_TIMEOUT)
  {
      yield();
  }
  if (!client.available())
  {
    Serial.println("Read timeout reached, dropping client");
    client.stop();
    return;
  }
  
  rest.handle(client);
 
}

