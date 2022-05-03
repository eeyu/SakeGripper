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
    EcatCommandInfo g1EcatCommandInfo = EcatCommandInfo();
    EcatCommandInfo g2EcatCommandInfo = EcatCommandInfo();
    EcatReplyInfo g1EcatReplyInfo = EcatReplyInfo();
    EcatReplyInfo g2EcatReplyInfo = EcatReplyInfo();

public:
    GripperManager() {    

    }

    void initialize() {  
        DEBUG_SERIAL.println("gripper initialized");
        dxl.begin(DXL_BAUD_RATE);
        dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

        gripper1 = GripperController(GRIPPER1_DXL_ID, &dxl);
        gripper2 = GripperController(GRIPPER2_DXL_ID, &dxl);
        gripper1.setZero(DEFAULT_GRIPPER1_ZERO);
        gripper2.setZero(DEFAULT_GRIPPER2_ZERO);
        dxl_timer.reset(0.01);
        DEBUG_SERIAL.println("Done");
    }

    void sendEcatCommandToGripper(EcatCommandInfo necatCommandInfo, int id) {
        if (id == 1)
        {
            g1EcatCommandInfo = necatCommandInfo;
        }
        else 
        {
            g2EcatCommandInfo = necatCommandInfo;
        }
    }

    EcatReplyInfo getEcatReplyInfoForGripper(int id) {
        if (id == 1)
        {
            return g1EcatReplyInfo;
        }
        return g2EcatReplyInfo;
        
    }

    void operate() {
        gripper1.sendEcatCommand(g1EcatCommandInfo);
        gripper2.sendEcatCommand(g2EcatCommandInfo);

        gripper1.doControl();
        gripper2.doControl();

        g1EcatReplyInfo = gripper1.getReplyInfo();
        g2EcatReplyInfo = gripper2.getReplyInfo();
    }

};

#endif
