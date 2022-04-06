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

    EcatCommandInfo g1EcatCommandInfo = EcatCommandInfo();
    EcatCommandInfo g2EcatCommandInfo = EcatCommandInfo();
    EcatReplyInfo g1EcatReplyInfo = EcatReplyInfo();
    EcatReplyInfo g2EcatReplyInfo = EcatReplyInfo();

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

            g1EcatCommandInfo = EcatCommandInfo(EASYCAT.BufferOut.Cust.g1Command,
                                                EASYCAT.BufferOut.Cust.g1Position,
                                                EASYCAT.BufferOut.Cust.g1Torque);
            g2EcatCommandInfo = EcatCommandInfo(EASYCAT.BufferOut.Cust.g2Command,
                                                EASYCAT.BufferOut.Cust.g2Position,
                                                EASYCAT.BufferOut.Cust.g2Torque);

            EASYCAT.BufferIn.Cust.g1Busy = g1EcatReplyInfo.busy;
            EASYCAT.BufferIn.Cust.g1Position = g1EcatReplyInfo.position;
            EASYCAT.BufferIn.Cust.g2Busy = g2EcatReplyInfo.busy;
            EASYCAT.BufferIn.Cust.g2Position = g2EcatReplyInfo.position;
        }
    }

    EcatCommandInfo getEcatCommandInfoForGripper(int id) {
        if (id == 1)
        {
            return g1EcatCommandInfo;
        }
        return g2EcatCommandInfo;
    }

    void setEcatReplyInfoForGripper(EcatReplyInfo necatReplyInfo, int id) {
        if (id == 1)
        {
            g1EcatReplyInfo = necatReplyInfo;
        }
        else 
        {
            g2EcatReplyInfo = necatReplyInfo;
        }
        
    }
};

#endif
