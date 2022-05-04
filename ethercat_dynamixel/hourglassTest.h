#ifdef TESTING
#include "Hourglass.h"

void testHourglass() {
    float maxTime = 5.0;
    Hourglass hourglass = Hourglass(maxTime);
    Serial.begin(9600);

    Serial.println("Run forward until out of time");
    hourglass.runForward();
    while (!hourglass.outOfTimeForward()) {
        delay(500);
        Serial.print(hourglass.getTimeLeftForwardSec()); Serial.print(" "); Serial.print(hourglass.getTimeLeftBackwardsSec());
        Serial.println();
    }
    Serial.println("Out of time forward.");
    Serial.print("Filling backwards: "); Serial.println(hourglass.lastEmptiedSideIsForward());

    Serial.println("Running backward until out of time.");
    hourglass.runBackward();
    while (!hourglass.outOfTimeBackward()) {
        delay(500);
        Serial.print(hourglass.getTimeLeftForwardSec()); Serial.print(" "); Serial.print(hourglass.getTimeLeftBackwardsSec());
        Serial.println();
    }
    Serial.println("Out of time backward.");
    Serial.print("Filling backwards: "); Serial.println(hourglass.lastEmptiedSideIsForward());

    Serial.println("Running forward, then pausing");
    hourglass.runForward();
    delay(1000.0 * maxTime / 2.0);
    hourglass.pause();
    Serial.print("Time at pause: "); Serial.println(hourglass.getTimeLeftForwardSec());
    delay(maxTime);
    Serial.print("Time at resume: "); Serial.println(hourglass.getTimeLeftForwardSec());
    
    Serial.println("Running backwards");
    hourglass.runBackward();
    while (!hourglass.outOfTimeBackward()) {
        delay(500);
        Serial.print(hourglass.getTimeLeftForwardSec()); Serial.print(" "); Serial.print(hourglass.getTimeLeftBackwardsSec());
        Serial.println();
    }
    Serial.print("Filling backwards: "); Serial.println(hourglass.lastEmptiedSideIsForward());
}

#endif