#ifndef __GRIPPER__
#define __GRIPPER__

// ECat Gripper interfaces between ecat and the LL gripper

#include <DynamixelShield.h>
#include "Timer.h"
#include "EcatCommunication.h"
#include "LowLevelGripper.h"

class GripperController {
private:
    LowLevelGripper llGripper;
    EcatCommandSignal lastCommandSignal = 0;
    EcatCommandInfo ecatCommandInfo = EcatCommandInfo();
    EcatReplyInfo ecatReplyInfo = EcatReplyInfo();

public:
    GripperController() {
    }

    GripperController(int dxlId, DynamixelShield *ndxl) {
        llGripper = LowLevelGripper(dxlId, ndxl);
    }

    void sendEcatCommand(EcatCommandInfo necatCommandInfo) {
        ecatCommandInfo = necatCommandInfo;
    }

    int isBusy() {
        return llGripper.isBusy();
    }

    void doControl() {
        llGripper.operate();
        bool newCommandWasSent = (lastCommandSignal == 0) && (ecatCommandInfo.signal != 0);

//        if (!llGripper.isBusy() && newCommandWasSent) {
        if (newCommandWasSent) {
            executeCommand(ecatCommandInfo);
        }
        lastCommandSignal = ecatCommandInfo.signal;
    }

private:
    void executeCommand(EcatCommandInfo ecatCommandInfo) {
        DEBUG_SERIAL.print("Sending to gripper: "); DEBUG_SERIAL.println(ecatCommandInfo.gripper_id);
        switch (ecatCommandInfo.signal) {
            case CALIBRATE:
                DEBUG_SERIAL.println("execute calibrate");
                llGripper.calibrate();
                break;
            case GOTO:
                llGripper.gotoPositionWithTorque(ecatCommandInfo.position, ecatCommandInfo.torque);
                DEBUG_SERIAL.println("execute goto");
                break;
            case RELEASE:
                llGripper.release();
                DEBUG_SERIAL.println("execute release");
                break;
            case OPEN:
                llGripper.open();
                DEBUG_SERIAL.println("execute open");
                break;
            default:
                break;
        }
    }
};

#endif
