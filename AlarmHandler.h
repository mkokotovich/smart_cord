#ifndef ALARMHANDLER_H
#define ALARMHANDLER_H

#include <Time.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <Arduino.h>

#define MAX_OPTIONS 5
#define MAX_ALARMS dtNBR_ALARMS //Need to set dtNBR_ALARMS in TimeAlarms.h, default is only 6

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

    void updateActiveAlarms(String &activeAlarms);
    String printAlarms();
    String digitalClockDisplay(time_t time);
    String printDigits(int digits);
private:
    void add(AlarmID_t id);
    void cancel(AlarmID_t id);

    AlarmID_t alarms[MAX_ALARMS];
    time_t next_alarm;
    bool paused;

};

// Global instance
extern AlarmHandler alarmHandler;

#endif
