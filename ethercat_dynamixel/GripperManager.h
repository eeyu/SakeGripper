#ifndef __GRIPPERMANAGER__
#define __GRIPPERMANAGER__

#include "GripperController.h"
#include "Parameters.h"
#include <DynamixelShield.h>
#include "Side.h"

// Class for 2 grippers on 1 dynamixel shield
// takes in an ethercat command and executes gripper actions accordingly

class GripperManager {
private:
    // Making lists on arduino is inconvenient
    GripperController gripperLeft;
    GripperController gripperRight;
    DynamixelShield *dxl;
    EcatCommandInfo leftEcatCommandInfo = EcatCommandInfo();
    EcatCommandInfo rightEcatCommandInfo = EcatCommandInfo();
    EcatReplyInfo leftEcatReplyInfo = EcatReplyInfo();
    EcatReplyInfo rightEcatReplyInfo = EcatReplyInfo();

public:
    GripperManager() {    

    }

    void initialize(DynamixelShield *ndxl) {  
        DEBUG_SERIAL.println("Gripper Manager Initializing");
        dxl = ndxl;
        dxl->begin(DXL_BAUD_RATE);
        dxl->setPortProtocolVersion(DXL_PROTOCOL_VERSION);

        gripperLeft = GripperController(LEFT_DXL_ID, dxl);
        gripperRight = GripperController(RIGHT_DXL_ID, dxl);
        gripperLeft.setZero(DEFAULT_LEFT_ZERO);
        gripperRight.setZero(DEFAULT_RIGHT_ZERO);

        if (dxl->ping(LEFT_DXL_ID) == true) {
            DEBUG_SERIAL.println("Left Gripper Found");
        } else {
            DEBUG_SERIAL.print("Left Gripper NOT Found");
        }
        if (dxl->ping(RIGHT_DXL_ID) == true) {
            DEBUG_SERIAL.println("Right Gripper Found");
        } else {
            DEBUG_SERIAL.print("Right Gripper NOT Found");
        }
    }

    void sendEcatCommandToGripper(EcatCommandInfo necatCommandInfo, Side side) {
        if (side == LEFT)
        {
            leftEcatCommandInfo = necatCommandInfo;
        }
        else 
        {
            rightEcatCommandInfo = necatCommandInfo;
        }
    }

    EcatReplyInfo getEcatReplyInfoForGripper(Side side) {
        if (side == LEFT)
        {
            return leftEcatReplyInfo;
        }
        return rightEcatReplyInfo;
        
    }

    void operate() {
        gripperLeft.sendEcatCommand(leftEcatCommandInfo);
        gripperRight.sendEcatCommand(rightEcatCommandInfo);

        gripperLeft.doControl();
        gripperRight.doControl();

        leftEcatReplyInfo = gripperLeft.getReplyInfo();
        rightEcatReplyInfo = gripperRight.getReplyInfo();
    }
};

#endif
