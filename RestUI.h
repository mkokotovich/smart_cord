// Needs to be from https://github.com/mkokotovich/aREST until pull request is merged in
#include <aREST.h>
// Needs to be from https://github.com/mkokotovich/aREST_UI until pull request is merged in
#include <aREST_UI.h>
#include "AlarmHandler.h"

#define READ_TIMEOUT 10000 //10 seconds

// Create aREST instance
aREST_UI rest = aREST_UI();

// The port to listen for UI
#define UI_LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(UI_LISTEN_PORT);

// Rest UI variables (declared in ino)
extern String current_state;
extern String activeAlarms;
extern String currentTime;

// Functions declared in ino
void powerOn();
void powerOff();

void saveRestUIToDisk(void)
{
    alarmHandler.saveAlarmsToDisk(current_state);
}

// Rest UI functions
int powerOn(String command)
{
    powerOn();
    saveRestUIToDisk();
    return 1;
}

int powerOff(String command)
{
    powerOff();
    saveRestUIToDisk();
    return 1;
}

int setAlarm(String command)
{
    String options[MAX_OPTIONS];
    int hour = 0;
    int minute = 0;
    int duration = 0;
    OnTick_t func = NULL;
    String func_action = String();

    Serial.println("setAlarm called with: " + command);

    if (!alarmHandler.parse_timer_string(command, options, hour, minute, duration))
    {
        activeAlarms = "Error parsing timer string";
        Serial.println(activeAlarms);
        return -1;
    }

    // Parse the options for smart_cord specific ones
    for (int i = 0; i < MAX_OPTIONS; i++)
    {
        if (options[i].equalsIgnoreCase("ON"))
        {
            func = powerOn;
            func_action += "On";
        }
        else if (options[i].equalsIgnoreCase("OFF"))
        {
            func = powerOff;
            func_action += "Off";
        }
        else if (options[i].equalsIgnoreCase("D"))
        {
            if (!alarmHandler.delete_timer(hour, minute))
            {
                activeAlarms = "Error deleting timer";
                Serial.println(activeAlarms);
                return -1;
            }
            saveRestUIToDisk();
            return 1;
        }
    }

    if (func == NULL)
    {
        activeAlarms = "Must supply ON or OFF as an option";
        Serial.println(activeAlarms);
        return -1;
    }

    if (!alarmHandler.add_new_timer(options, hour, minute, duration, func, func_action))
    {
        activeAlarms = "Error adding timer";
        Serial.println(activeAlarms);
        return -1;
    }

    saveRestUIToDisk();

    return 1;
}

int cancelAllTimers(String command)
{
    Serial.println("cancelAllTimers called");
    alarmHandler.cancelAllAlarms();
    saveRestUIToDisk();
    return 1;
}

int pauseAllTimers(String command)
{
    Serial.println("pauseAllTimers called");
    alarmHandler.pauseAllAlarms();
    saveRestUIToDisk();
    return 1;
}

void setupRestUI()
{
    // Give name and ID to device
    rest.set_id("1");
    rest.set_name("esp8266");
    rest.title("Lights");

    // Display the current state
    rest.variable_label("current_state", "Current state of smart cord", &current_state);

    // Register the functions with the rest UI

    rest.function_button("powerOn", "Power On", powerOn);
    rest.function_button("powerOff", "Power Off", powerOff);

    rest.label("Add an alarm:");
    rest.function_with_input_button("setAlarm", "Add", setAlarm);
    rest.label("For example: 10:15 PM off, or 7:00 am on, or 15 on (in 15 minutes turn it on)", true);
    // Display current time
    rest.variable_label("currentTime", "Current time", &currentTime);
    // Display the timer state
    rest.variable_label("activeAlarms", "Active Alarms", &activeAlarms);
    rest.function_button("pauseAllTimers", "Pause/Resume All Timers", pauseAllTimers);
    rest.function_button("cancelAllTimers", "Cancel All Timers", cancelAllTimers);

}

void startRestUIServer(void)
{
    // Start the server
    server.begin();
    Serial.println("Rest UI server started on port: " + String(UI_LISTEN_PORT));
    alarmHandler.loadAlarmsFromDisk(current_state, powerOff, powerOn);
    if (current_state.equals("Off"))
    {
        powerOff();
    }
    else if (current_state.equals("ON"))
    {
        powerOn();
    }
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
