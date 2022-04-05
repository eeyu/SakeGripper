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
    EcatCommandSignal command;
    uint8_t position;
    uint8_t torque;

    EcatCommandInfo() {
        command = WAITING;
    }

    EcatCommandInfo(uint8_t nCommand, uint8_t nPosition, uint8_t nTorque) {
        if (nCommand > MAX_SIGNAL_VAL || nCommand < 0) {
            nCommand = 0;
        }
        command = static_cast<EcatCommandSignal>(nCommand);
        position = constrain(nPosition, GRIPPER_SCALE_MIN, GRIPPER_SCALE_MAX);
        torque = constrain(nTorque, GRIPPER_SCALE_MIN, GRIPPER_SCALE_MAX);
    }
};

struct EcatReplyInfo {
    uint8_t busy;
    uint8_t position;
};

#endif