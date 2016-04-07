// Needs to be from https://github.com/mkokotovich/aREST until pull request is merged in
#include <aREST.h>
// Needs to be from https://github.com/mkokotovich/aREST_UI until pull request is merged in
#include <aREST_UI.h>
#include "AlarmHandler.h"

#define READ_TIMEOUT 10000 //10 seconds

// Create aREST instance
aREST_UI rest = aREST_UI();

// Create AlarmHandler
AlarmHandler alarmHandler = alarmHandler();

// The port to listen for UI
#define UI_LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(UI_LISTEN_PORT);


// Rest UI variables (declared in ino)
extern String current_state;
String activeTimers = "None";


// Functions declared in ino
void powerOn();
void powerOff();

// Rest UI functions
int powerOn(String command)
{
    powerOn();
    return 1;
}

int powerOff(String command)
{
    powerOff();
    return 1;
}


int setTimer(String command)
{
    String options[MAX_OPTIONS];
    int hour = 0;
    int minute = 0;
    int duration = 0;

    Serial.println("setTimer called with: " + command);

    alarmHandler.parse_timer_string(command, options, hour, minute, duration);

    alarmHandler.add_new_timer(options, hour, minute, duration);

    activeTimers += command;

    return 1;
}

int cancelAllTimers(String command)
{
    alarmHandler.cancelAllTimers();
    activeTimers = "None";
    return 1;
}

int pauseAllTimers(String command)
{
    activeTimers += "Paused";
    return 1;
}

void setupRestUI()
{
    // Give name and ID to device
    rest.set_id("1");
    rest.set_name("esp8266");
    rest.title("Deck Lights");

    // Display the current state
    rest.variable_label("current_state", "Current state of smart cord", &current_state);

    // Register the functions with the rest UI

    rest.function_button("powerOn", "Power On", powerOn);
    rest.function_button("powerOff", "Power Off", powerOff);

    rest.label("Add a timer:");
    rest.function_with_input_button("setTimer", "Add", setTimer);
    // Display the timer state
    rest.variable_label("activeTimers", "Active Timers", &activeTimers);
    rest.function_button("pauseAllTimers", "Pause/Resume All Timers", pauseAllTimers);
    rest.function_button("cancelAllTimers", "Cancel All Timers", cancelAllTimers);

}

void startRestUIServer(void)
{
  // Start the server
  server.begin();
  Serial.println("Rest UI server started on port: " + String(UI_LISTEN_PORT));
}

void handleRestUI(void)
{
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
