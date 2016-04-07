#include "AlarmHandler.h"

AlarmHandler::AlarmHandler():
    num_alarms(0)
{
    for (int i = 0; i < MAX_ALARMS; i++)
    {
        alarms[i] = 0;
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
                    i++; // skip the :
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
                if (command.charAt(i) < '0' || command.charAt(i) > '9')
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

void AlarmHandler::add_new_timer(
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

    if (duration == 0)
    {
        // Alarm
        if (repeating)
        {
            id = Alarm.alarmRepeat(hour, minute, 0, func);
        }
        else
        {
            id = Alarm.alarmOnce(hour, minute, 0, func);
        }
    }
    else
    {
        // Timer
        id = Alarm.timerOnce(duration*60, func);
    }

    add(id);
}

void AlarmHandler::cancelAllAlarms()
{
    for (int i = 0; i < num_alarms; i++)
    {
        Alarm.disable(alarms[i]);
    }
    num_alarms = 0;
    paused = false;
}

void AlarmHandler::pauseAllAlarms()
{
    for (int i = 0; i < num_alarms; i++)
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
    paused = !paused;
}

void AlarmHandler::add(AlarmID_t id)
{
    alarms[num_alarms] = id;
    num_alarms++;
}
