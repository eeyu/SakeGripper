#ifndef __ECATMANAGER__
#define __ECATMANAGER__

#include "EcatCommunication.h"
#include "GripperECAT.h"
#include "EasyCAT.h"                // EasyCAT library to interface the LAN9252
#include "Timer.h"


class EcatManager {
private:
    EasyCAT EASYCAT;            
    Timer ecat_timer;

    EcatCommandInfo ecatCommandInfo = EcatCommandInfo();
    EcatReplyInfo ecatReplyInfo = EcatReplyInfo();

public:
    EcatManager() {

    }

    void initialize() {
        DEBUG_SERIAL.print ("\nEasyCAT - Generic EtherCAT slave\n");          // print the banner
                                                                    //---- initialize the EasyCAT board -----                                       
        if (EASYCAT.Init() == true){                                                               
            DEBUG_SERIAL.print ("initialized");                                
        } else {                                                               
            DEBUG_SERIAL.print ("initialization failed");                        
        }
        ecat_timer.usePrecision();
        ecat_timer.reset(0.001);
    }

    void operate() {
        EASYCAT.MainTask();
        if (ecat_timer.timeOut())                    
        {                                                     
            ecat_timer.reset();

            ecatCommandInfo = EcatCommandInfo(EASYCAT.BufferOut.Cust.gripper_id,
                                                EASYCAT.BufferOut.Cust.command_signal,
                                                EASYCAT.BufferOut.Cust.position,
                                                EASYCAT.BufferOut.Cust.torque);

            EASYCAT.BufferIn.Cust.gripper1_busy = ecatReplyInfo.gripper1_busy;
            EASYCAT.BufferIn.Cust.gripper2_busy = ecatReplyInfo.gripper2_busy;
        }
    }

    EcatCommandInfo getEcatCommandInfo() {
        return ecatCommandInfo;
    }

    void setEcatReplyInfo(EcatReplyInfo necatReplyInfo) {
        ecatReplyInfo = necatReplyInfo;
    }
};

#endif
