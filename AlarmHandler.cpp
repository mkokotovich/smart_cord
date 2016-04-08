#include "AlarmHandler.h"

// Global
AlarmHandler alarmHandler = AlarmHandler();

AlarmHandler::AlarmHandler():
    paused(false)
{
    for (int i = 0; i < MAX_ALARMS; i++)
    {
        alarms[i] = dtINVALID_ALARM_ID;
    }
}

bool AlarmHandler::parse_timer_string(String command,
        String options[],
        int &hour,
        int &minute,
        int &duration)
{
    String temp = "";
    int current_option = 0;
    bool success = false;

    parse_state state = unknown;

    for (int i = 0; i < command.length(); i++)
    {
        // Really simple url decoding, just get rid of it
        if (command.charAt(i) == '%')
        {
            command.setCharAt(i, ' ');
            command.setCharAt(i+1, ' ');
            command.setCharAt(i+2, ' ');
            i+=2;
        }
    }

    for (int i = 0; i < command.length(); i++)
    {
        switch (state)
        {
            case option:
                if (command.charAt(i) == ' ')
                {
                    // End of option
                    if (current_option == MAX_OPTIONS)
                    {
                        // Just skip it
                        continue;
                    }
                    options[current_option++] = temp;
                    temp = "";
                    state = unknown;
                }
                else
                {
                    temp += command.charAt(i);
                }
                break;
            case time_unknown:
                if (command.charAt(i) == ':')
                {
                    // End of the hour
                    hour = temp.toInt();
                    state = time_minute;
                    temp = "";
                }
                else if (command.charAt(i) < '0' || command.charAt(i) > '9')
                {
                    // Reached the end of the time without hitting :, must be duration
                    duration = temp.toInt();
                    success = true;
                    state = option;
                    temp = "";
                    temp += command.charAt(i);
                }
                else
                {
                    temp += command.charAt(i);
                }
                break;
            case time_minute:
                if (command.charAt(i) < '0' || command.charAt(i) > '9')
                {
                    // Reached the end of the minute
                    minute = temp.toInt();
                    state = time_ampm;
                    temp = "";
                    i--; // Re-process this character in ampm state
                }
                else
                {
                    temp += command.charAt(i);
                }
                break;
            case time_ampm:
                if (command.length() >= i+2)
                {
                    String sub = command.substring(i, i+2);
                    if (sub.equalsIgnoreCase("AM"))
                    {
                        // No need to adjust hour
                        i += 2;
                        success = true;
                        state = unknown;
                    }
                    else if (sub.equalsIgnoreCase("PM"))
                    {
                        hour += 12;
                        i += 2;
                        success = true;
                        state = unknown;
                    }
                    // Otherwise, there could be a space before the AM/PM. Keep parsing
                }
                else
                {
                    state = unknown;
                    i--; //Re-process this character
                }
                break;
            case unknown:
                if (command.charAt(i) == ' ')
                {
                    // Do nothing, skip the space
                }
                else if (command.charAt(i) < '0' || command.charAt(i) > '9')
                {
                    // Not a number, must be beginning of an option
                    state = option;
                    temp = "";
                    temp += command.charAt(i);
                }
                else if (command.charAt(i) >= '0' && command.charAt(i) <= '9')
                {
                    state = time_unknown;
                    temp = "";
                    temp += command.charAt(i);
                }
                break;
        }
    }
    // Now handle the last temp buffer, if needed
    switch (state)
    {
        case option:
            // End of option
            if (current_option != MAX_OPTIONS)
            {
                options[current_option++] = temp;
            }
            break;
        case time_unknown:
            // Reached the end of the time without hitting :, must be duration
            duration = temp.toInt();
            success = true;
            break;
    }

    if (success != true)
    {
        Serial.println("Unable to parse: " + command);
    }
    else
    {
        if (duration != 0)
        {
            Serial.println("Parsed duration: " + String(duration));
        }
        else
        {
            Serial.println("Parsed time: " + String(hour) + ":" + String(minute));
        }
    }

    return success;
}

bool AlarmHandler::add_new_timer(
        String options[],
        int &hour,
        int &minute,
        int &duration)
{
    OnTick_t func = NULL;
    bool repeating = false;
    AlarmID_t id = 0;

    // Parse the options
    for (int i = 0; i < MAX_OPTIONS; i++)
    {
        if (options[i].equalsIgnoreCase("ON"))
        {
            func = powerOn;
        }
        else if (options[i].equalsIgnoreCase("OFF"))
        {
            func = powerOff;
        }
        else if (options[i].equalsIgnoreCase("R"))
        {
            repeating = true;
        }
    }

    if (func == NULL)
    {
        Serial.println("Must supply ON or OFF as an option");
        return false;
    }

    if (duration == 0)
    {
        time_t alarm_time = 0;

        alarm_time = hour * SECS_PER_HOUR + minute * SECS_PER_MIN;

        // Alarm
        if (repeating)
        {
            id = Alarm.alarmRepeat(alarm_time, func);
        }
        else
        {
            id = Alarm.alarmOnce(alarm_time, func);
        }
    }
    else
    {
        // Timer
        id = Alarm.timerOnce(duration*60, func);
    }

    if (id == dtINVALID_ALARM_ID)
    {
        Serial.println("Error adding alarm");
        return false;
    }

    return add(id);
}

void AlarmHandler::cancelAllAlarms()
{
    for (int i = 0; i < MAX_ALARMS; i++)
    {
        if (alarms[i] != dtINVALID_ALARM_ID)
        {
            cancel(alarms[i]);
            alarms[i] = dtINVALID_ALARM_ID;
        }
    }
    paused = false;
}

void AlarmHandler::pauseAllAlarms()
{
    for (int i = 0; i < MAX_ALARMS; i++)
    {
        if (alarms[i] != dtINVALID_ALARM_ID)
        {
            if (paused == true)
            {
                Alarm.enable(alarms[i]);
            }
            else
            {
                Alarm.disable(alarms[i]);
            }
        }
    }
    paused = !paused;
}

bool AlarmHandler::add(AlarmID_t id)
{
    int i = 0;
    for (i = 0; i < MAX_ALARMS; i++)
    {
        if (alarms[i] == dtINVALID_ALARM_ID)
        {
            alarms[i] = id;
            break;
        }
    }
    if (i == MAX_ALARMS)
    {
        Serial.println("Already have used all available alarms");
        return false;
    }
    return true;
}

void AlarmHandler::cancel(AlarmID_t id)
{
    Alarm.disable(id);
    Alarm.free(id);
}

void AlarmHandler::updateActiveAlarms(String &activeAlarms)
{
    if (Alarm.count() == 0)
    {
        activeAlarms = "None";
    }
    else
    {
        activeAlarms = printAlarms();
    }
}

String AlarmHandler::printAlarms()
{
    time_t active_alarms[MAX_ALARMS];
    int current_alarm = 0;

    for (int i = 0; i < MAX_ALARMS; i++)
    {
        if (alarms[i] != dtINVALID_ALARM_ID)
        {
            dtAlarmPeriod_t alarm_type = Alarm.readType(alarms[i]);
            if (alarm_type != dtTimer)
            {
                time_t alarm_time = Alarm.read(alarms[i]);
                if (alarm_time == dtINVALID_TIME)
                {
                    // Remove all alarms that are no longer active
                    // They don't need to be cancelled, that happens automatically
                    alarms[i] = dtINVALID_ALARM_ID;
                    // TODO: this isn't happening
                    Serial.println("removed alarm");
                }
                else
                {
                    active_alarms[current_alarm] = alarm_time;
                    current_alarm++;
                }
            }
        }
    }
    
    if (current_alarm == 0)
    {
        return String();
    }

    // Code copied from Running Median example
    // sort current_alarm
    for (uint8_t i=0; i< current_alarm-1; i++)
    {
        uint8_t m = i;
        for (uint8_t j=i+1; j< current_alarm; j++)
        {
            if (active_alarms[j] < active_alarms[m]) m = j;
        }
        if (m != i)
        {
            time_t t = active_alarms[m];
            active_alarms[m] = active_alarms[i];
            active_alarms[i] = t;
        }
    }

    String output = digitalClockDisplay(active_alarms[0]);
    for (int i = 1; i < current_alarm; i++)
    {
        output += String(", ") + digitalClockDisplay(active_alarms[i]);
    }

    Serial.println("printAlarms: ");
    Serial.println(output);
    return output;
}


String AlarmHandler::digitalClockDisplay(time_t time)
{
    return String(hourFormat12(time)) + printDigits(minute(time)) + (isAM(time) ? " AM" : " PM");
}

String AlarmHandler::printDigits(int digits)
{
    // utility for digital clock display: prints preceding colon and leading 0
    String output = ":";
    if(digits < 10)
        output += "0";
    output += String(digits);

    return output;
}

