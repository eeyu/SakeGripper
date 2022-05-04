#ifndef __ECATMANAGER__
#define __ECATMANAGER__

#include "EcatCommunication.h"
#include "GripperECAT.h"
#include "EasyCAT.h"                // EasyCAT library to interface the LAN9252
#include "Timer.h"
#include "Side.h"


class EcatManager {
private:
    EasyCAT EASYCAT;            

    EcatCommandInfo leftEcatCommandInfo = EcatCommandInfo();
    EcatCommandInfo rightEcatCommandInfo = EcatCommandInfo();
    EcatReplyInfo leftEcatReplyInfo = EcatReplyInfo();
    EcatReplyInfo rightEcatReplyInfo = EcatReplyInfo();

public:
    EcatManager() {

    }

    void initialize() {
        DEBUG_SERIAL.print ("\nEasyCAT - Generic EtherCAT slave\n");          // print the banner
                                                                    //---- initialize the EasyCAT board -----                                       
        if (EASYCAT.Init() == true){                                                               
            DEBUG_SERIAL.println ("EasyCAT Initialized");                                
        } else {                                                               
            DEBUG_SERIAL.println ("EasyCAT Initialization FAILED");                        
        }
    }

    void operate() {
        EASYCAT.MainTask();

        leftEcatCommandInfo = EcatCommandInfo(EASYCAT.BufferOut.Cust.leftCommand,
                                            EASYCAT.BufferOut.Cust.leftPosition,
                                            EASYCAT.BufferOut.Cust.leftTorque);
        rightEcatCommandInfo = EcatCommandInfo(EASYCAT.BufferOut.Cust.rightCommand,
                                            EASYCAT.BufferOut.Cust.rightPosition,
                                            EASYCAT.BufferOut.Cust.rightTorque);

        EASYCAT.BufferIn.Cust.leftBusy =        leftEcatReplyInfo.busy;
        EASYCAT.BufferIn.Cust.leftPosition =    leftEcatReplyInfo.position;
        EASYCAT.BufferIn.Cust.leftTorque =      leftEcatReplyInfo.torque;
        EASYCAT.BufferIn.Cust.leftTemperature = leftEcatReplyInfo.temperature;
        EASYCAT.BufferIn.Cust.leftError =       leftEcatReplyInfo.error;

        EASYCAT.BufferIn.Cust.rightBusy =       rightEcatReplyInfo.busy;
        EASYCAT.BufferIn.Cust.rightPosition =   rightEcatReplyInfo.position;
        EASYCAT.BufferIn.Cust.rightTorque =     rightEcatReplyInfo.torque;
        EASYCAT.BufferIn.Cust.rightTemperature = rightEcatReplyInfo.temperature;
        EASYCAT.BufferIn.Cust.rightError =      rightEcatReplyInfo.error;
    }

    EcatCommandInfo getEcatCommandInfoForGripper(Side side) {
        if (side == LEFT)
        {
            return leftEcatCommandInfo;
        }
        return rightEcatCommandInfo;
    }

    void setEcatReplyInfoForGripper(EcatReplyInfo necatReplyInfo, Side side) {
        if (side == LEFT)
        {
            leftEcatReplyInfo = necatReplyInfo;
        }
        else 
        {
            rightEcatReplyInfo = necatReplyInfo;
        }
        
    }
};

#endif
