#include "Timer.h"


Timer::Timer()
{
    reset();
}

Timer::Timer(unsigned long duration_ms)
{
    set(duration_ms);
}

bool Timer::isActive()
{
    return m_start_ms != 0;
}

void Timer::deactivate()
{
    m_start_ms = 0;
}

bool Timer::isExpired()
{
    String ignore;
    return isExpired(ignore);
}

bool Timer::isExpired(String &state)
{
    bool expired = false;
    if (isActive())
    {
	unsigned long ms_elapsed;
	if ((ms_elapsed = (millis() - m_start_ms)) > m_duration_ms)
	{
	    expired = true;
	    m_state = String("Expired");
	    deactivate();
	}
	else if (ms_elapsed > STATE_UPDATE_INTERVAL + m_state_last_updated)
	{
	    m_state_last_updated = ms_elapsed;
	    m_state = String("Enabled, ") + ceil(((float)(m_duration_ms - ms_elapsed))/1000/60) + String(" minutes left");
	}
	state = m_state;
    }
    return expired;
}

void Timer::set(unsigned long duration_ms)
{
    String ignore;
    set(duration_ms, ignore);
}

void Timer::set(unsigned long duration_ms, String &state)
{
    m_start_ms = millis();
    m_duration_ms = duration_ms;
    m_state_last_updated = 0;
    m_state = String("Enabled, ") + ceil((float)duration_ms/1000/60) + String(" minutes left");
    state = m_state;
}

void Timer::set(String timestr)
{
    String ignore;
    set(timestr, ignore);
}

void Timer::set(String timestr, String &state)
{
    // timestr should be something like 7:00 PM
    // First parse string into time
}

void Timer::cancel()
{
    String ignore;
    cancel(ignore);
}

void Timer::cancel(String &state)
{
    reset();
    state = m_state;
}

void Timer::reset()
{
    m_start_ms = 0;
    m_duration_ms = 0;
    m_state_last_updated = 0;
    m_state = "Disabled";
}

String Timer::getState()
{
    return m_state;
}

