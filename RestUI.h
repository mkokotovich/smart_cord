// Needs to be from https://github.com/mkokotovich/aREST until pull request is merged in
#include <aREST.h>
// Needs to be from https://github.com/mkokotovich/aREST_UI until pull request is merged in
#include <aREST_UI.h>

#define READ_TIMEOUT 10000 //10 seconds

// Defined in .ino
void controlRelay(int state);
extern String current_state;
extern String onTimer_state;
extern String offTimer_state;
extern Timer onTimer;
extern Timer offTimer;

// Create aREST instance
aREST_UI rest = aREST_UI();

// The port to listen for UI
#define UI_LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(UI_LISTEN_PORT);


// Rest UI variables

// Rest UI functions

int powerOn(String command)
{
    controlRelay(LOW);
    current_state = "On";
    return 1;
}

int powerOff(String command)
{
    controlRelay(HIGH);
    current_state = "Off";
    return 1;
}

int onTimerFunc(String command)
{
    Serial.print("onTimerFunc called with: ");
    Serial.println(command);

    // Passed to us in minutes, need to convert
    unsigned long timer_ms_duration = strtoul(command.c_str(), NULL, 10) * 60 * 1000;
    onTimer.set(timer_ms_duration, onTimer_state);

    return 1;
}

int offTimerFunc(String command)
{
    Serial.print("offTimerFunc called with: ");
    Serial.println(command);

    // Passed to us in minutes, need to convert
    unsigned long timer_ms_duration = strtoul(command.c_str(), NULL, 10) * 60 * 1000;
    offTimer.set(timer_ms_duration, offTimer_state);

    return 1;
}

int cancelOnTimer(String command)
{
    onTimer.cancel(onTimer_state);
    return 1;
}

int cancelOffTimer(String command)
{
    offTimer.cancel(offTimer_state);
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
