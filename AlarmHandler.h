#include <Time.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <Arduino.h>

#define MAX_OPTIONS 5
#define MAX_ALARMS 20

// Rest UI variables
extern String current_state;
extern String activeTimers;

// Alarm functions (defined in .ino)

void powerOn();
void powerOff();

typedef enum {
    unknown = 0,
    option,
    time_unknown,
    time_minute,
    time_ampm
} parse_state;

class AlarmHandler {

public:
    AlarmHandler();

    bool parse_timer_string(String command,
        String options[],
        int &hour,
        int &minute,
        int &duration);

    void add_new_timer(
        String options[],
        int &hour,
        int &minute,
        int &duration);

    void pauseAllAlarms();
    void cancelAllAlarms();
private:
    void add(AlarmID_t id);

    AlarmID_t alarms[MAX_ALARMS];
    int num_alarms;
    bool paused;

};
