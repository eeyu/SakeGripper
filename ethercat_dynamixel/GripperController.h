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

    EcatReplyInfo getReplyInfo() {
        ecatReplyInfo.busy = isBusy();
        ecatReplyInfo.position = getPosition();
        return ecatReplyInfo;
    }

    int isBusy() {
        return llGripper.isBusy();
    }

    double getPosition() {
        return llGripper.getPosition();
    }

    void doControl() {
        llGripper.operate();
        bool newCommandWasSent = (lastCommandSignal != ecatCommandInfo.command) 
            && (ecatCommandInfo.command != EcatCommandSignal::WAITING);

//        if (!llGripper.isBusy() && newCommandWasSent) {
        if (newCommandWasSent || ecatCommandInfo.command == EcatCommandSignal::GOTO) {
            executeCommand(ecatCommandInfo);
        }
        lastCommandSignal = ecatCommandInfo.command;
    }

    void setZero(int zero) {
        llGripper.setZero(zero);
    }

private:
    void executeCommand(EcatCommandInfo ecatCommandInfo) {
        switch (ecatCommandInfo.command) {
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
