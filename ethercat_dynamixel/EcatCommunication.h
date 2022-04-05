#ifndef __ECATCOMM__
#define __ECATCOMM__

#include "Gripper.h"

enum EcatCommandSignal {
    WAITING = 0,
    CALIBRATE = 1,
    GOTO = 2,
    RELEASE = 3,
    OPEN = 4
};
#define MAX_SIGNAL_VAL 4

struct EcatCommandInfo {
    EcatCommandSignal signal;
    uint8_t position;
    uint8_t torque;

    EcatCommandInfo() {
    }

    EcatCommandInfo(uint8_t nsignal, uint8_t nposition, uint8_t ntorque) {
        if (nsignal > MAX_SIGNAL_VAL || nsignal < 0) {
            nsignal = 0;
        }
        signal = static_cast<EcatCommandSignal>(nsignal);
        position = constrain(nposition, GRIPPER_SCALE_MIN, GRIPPER_SCALE_MAX);
        torque = constrain(ntorque, GRIPPER_SCALE_MIN, GRIPPER_SCALE_MAX);
    }
};

struct EcatReplyInfo {
    uint8_t gripper_busy;
};

#endif