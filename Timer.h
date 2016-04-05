#ifndef _TIMER_
#define _TIMER_

#include <Arduino.h>
//#define String string

#define STATE_UPDATE_INTERVAL 1000

class Timer {
    public:
        Timer();
        Timer(unsigned long duration_ms);

        ~Timer() {};

        bool isActive();
        bool isExpired();
        bool isExpired(String &state);

        void set(unsigned long duration_ms);
        void set(unsigned long duration_ms, String &state);
        void set(String timestr);
        void set(String timestr, String &state);
	
        void cancel();
        void cancel(String &state);

        void reset();

	String getState();
        
    private:
        void deactivate();

        unsigned long m_start_ms;
        unsigned long m_duration_ms;
        unsigned long m_state_last_updated;

	String m_state;

};

#endif // TIMER
