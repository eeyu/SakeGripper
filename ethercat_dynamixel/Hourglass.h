#ifndef __HOURGLASS__
#define __HOURGLASS__

class Hourglass
{
public:
    Hourglass() {}

    Hourglass(float maxTime) 
    {
        maxTime_u = convertSecToUnit(maxTime);
        reset();
    }

    void runForward()
    {
        if (state != FORWARD) 
        {
            timeLeftForwardAtLastStateChange_u = getTimeLeftForward_u();
            startTimeOfLastStateChangeAbsolute_u = getAbsoluteTime_u();
            state = FORWARD;
        }
    }

    void runBackward()
    {
        if (state != BACKWARD) 
        {
            timeLeftForwardAtLastStateChange_u = getTimeLeftForward_u();
            startTimeOfLastStateChangeAbsolute_u = getAbsoluteTime_u();
            state = BACKWARD;
        }
    }

    void pause()
    {
        if (state != PAUSE)
        {
            timeLeftForwardAtLastStateChange_u = getTimeLeftForward_u();
            startTimeOfLastStateChangeAbsolute_u = getAbsoluteTime_u();
            state = PAUSE;
        }
    }

    void reset()
    {
        timeLeftForwardAtLastStateChange_u = maxTime_u;
        state = PAUSE;
        fillingDirection = FORWARD;
    }

    float getTimeLeftForwardSec()
    {
        return convertUnitToSec(getTimeLeftForward_u());
    }

    float getTimeLeftBackwardsSec()
    {
        long timeLeftBackward_u = maxTime_u - getTimeLeftForward_u();
        return convertUnitToSec(timeLeftBackward_u);
    }

    bool outOfTimeForward()
    {
        return getTimeLeftForward_u() == 0;
    }

    bool outOfTimeBackward()
    {
        return getTimeLeftForward_u() == maxTime_u;
    }

    bool lastEmptiedSideIsForward()
    {
        return fillingDirection == BACKWARD;
    }

private:
    enum Direction
    {
        FORWARD,
        BACKWARD,
        PAUSE
    };

    long maxTime_u;
    long timeLeftForwardAtLastStateChange_u = 0;
    long startTimeOfLastStateChangeAbsolute_u = 0;
    Direction state = PAUSE;
    Direction fillingDirection = FORWARD;

    long getTimeLeftForward_u()
    {
        long timeLeftForward_u;
        long timeElapsedUnit;
        switch (state)
        {
            case FORWARD:
                timeElapsedUnit = getAbsoluteTime_u() - startTimeOfLastStateChangeAbsolute_u;
                timeLeftForward_u = timeLeftForwardAtLastStateChange_u - timeElapsedUnit;
                if (timeLeftForward_u < 0)
                {
                    timeLeftForward_u = 0;
                    fillingDirection = BACKWARD;
                }
                break;
            case PAUSE:
                timeLeftForward_u = timeLeftForwardAtLastStateChange_u;
                break;
            case BACKWARD:
                timeElapsedUnit = getAbsoluteTime_u() - startTimeOfLastStateChangeAbsolute_u;
                timeLeftForward_u = timeLeftForwardAtLastStateChange_u + timeElapsedUnit;
                if (timeLeftForward_u > maxTime_u)
                {
                    timeLeftForward_u = maxTime_u;
                    fillingDirection = FORWARD;
                }
                break;
            default:
                break;
        }
        return timeLeftForward_u;
    }

    long getAbsoluteTime_u() 
    {
        return micros();
    }

    long convertSecToUnit(float t)
    {
        return (long) (t * 1000000);
    }

    float convertUnitToSec(long t) 
    {
        return (float) (t / 1000000.0);
    }

};

#endif