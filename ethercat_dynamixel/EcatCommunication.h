#ifndef __ECATCOMM__
#define __ECATCOMM__

#include "Parameters.h"

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
    uint8_t gripper_id;

    EcatCommandInfo() {
        signal = WAITING;
    }

    EcatCommandInfo(uint8_t ngripper_id, uint8_t nsignal, uint8_t nposition, uint8_t ntorque) {
        if (nsignal > MAX_SIGNAL_VAL || nsignal < 0) {
            nsignal = 0;
        }
        gripper_id = ngripper_id;
        signal = static_cast<EcatCommandSignal>(nsignal);
        position = constrain(nposition, GRIPPER_SCALE_MIN, GRIPPER_SCALE_MAX);
        torque = constrain(ntorque, GRIPPER_SCALE_MIN, GRIPPER_SCALE_MAX);
    }
};

struct EcatReplyInfo {
    uint8_t gripper1_busy;
    uint8_t gripper2_busy;
};

#endif