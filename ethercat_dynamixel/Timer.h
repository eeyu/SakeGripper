#ifndef __TIMER__
#define __TIMER__

#include <arduino.h>

#define TIME_INFINITE -1

// TODO: cant do Timer a; a = Timer(xxx); or an error will happen...not sure why
// Used to time the length of functions or create a delay.
// Can be set to check milliseconds or microseconds.
// Interaction is always in seconds
struct Timer {
private:
    unsigned long time_start;
    unsigned long time_finish;
    unsigned long (*checkTime)();
    bool using_precision;

    bool is_turned_off;
public:
    Timer(unsigned long ntime_finish=0) {
        checkTime = millis;
        using_precision = false;
        is_turned_off = true;
        set(ntime_finish);   
    }

    void usePrecision()  {
        checkTime = micros;
        is_turned_off = false;
        using_precision = true;
        restart();
    }

    // Starts the timer to run for t ms.
    void set(float t) {
        time_start = checkTime();
        if (using_precision) {
            time_finish = (unsigned long) (t * 1000000);
        } else {
            time_finish = (unsigned long) (t * 1000);
        }
    }

    void restart() {
        set(time_finish);
    }

    // void setIfTurnedOff(float t) {
    //     if (is_turned_off) {
    //         set(t);
    //     }
    // }

    // void restartIfTurnedOff() {
    //     if (is_turned_off) {
    //         restart();
    //     }
    // }



    // Returns whether the time has elapsed
    bool isRinging() {
        if (checkTimeLeft() <= 0.0 ) {
            return true;
        } else {
            return false;
        }
    }

    void stopRinging() {
        is_turned_off = true;
    }

    bool isTickingDown() {
        return !is_turned_off;
    }

    float dt() {
        if (using_precision) {
            return (checkTime() - time_start) / 1000000.0;
        } else {
            return (checkTime() - time_start) / 1000.0;
        }
    }

    float checkTimeLeft() {
        return (time_start + time_finish - checkTime());
    }
};

#endif