#ifndef __GRIPPERMANAGER__
#define __GRIPPERMANAGER__

#include "GripperController.h"
#include "Parameters.h"
#include <DynamixelShield.h>

// Class for 2 grippers on 1 dynamixel shield
// takes in an ethercat command and executes gripper actions accordingly

class GripperManager {
private:
    // Making lists on arduino is inconvenient
    GripperController gripper1;
    GripperController gripper2;
    DynamixelShield dxl;
    Timer dxl_timer;
    EcatCommandInfo ecatCommandInfo = EcatCommandInfo();
    EcatReplyInfo ecatReplyInfo = EcatReplyInfo();

public:
    GripperManager() {    

    }

    void initialize() {  
        DEBUG_SERIAL.println("gripper initialized");
        dxl.begin(DXL_BAUD_RATE);
        dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
        gripper1 = GripperController(GRIPPER1_DXL_ID, &dxl);
        gripper2 = GripperController(GRIPPER2_DXL_ID, &dxl);
        dxl_timer.reset(0.01);
    }

    void sendEcatCommand(EcatCommandInfo necatCommandInfo) {
        ecatCommandInfo = necatCommandInfo;
    }

    EcatReplyInfo getEcatReplyInfo() {
        return ecatReplyInfo;
    }

    void operate() {
        sendCommandToAppropriateGripper(ecatCommandInfo);
        gripper1.doControl();
        gripper2.doControl();
        ecatReplyInfo.gripper1_busy = gripper1.isBusy();
        ecatReplyInfo.gripper2_busy = gripper2.isBusy();
    }
private:
    void sendCommandToAppropriateGripper(EcatCommandInfo ecatCommandInfo) {
        int gripperId = ecatCommandInfo.gripper_id;
        switch (gripperId) {
            case 1:
                gripper1.sendEcatCommand(ecatCommandInfo);
                break;
            case 2:
                gripper2.sendEcatCommand(ecatCommandInfo);
                break;
            default:
                break;
        }
    }
};

#endif