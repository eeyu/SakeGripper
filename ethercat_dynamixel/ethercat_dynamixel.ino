#include <SPI.h>                    // SPI library
#include "Timer.h"

// #define TESTING

#ifdef TESTING
#include "hourglassTest.h"
#endif

#define CUSTOM
#include "Side.h"
#include "Parameters.h"
#include "GripperManager.h"
#include "EcatManager.h"
#include "EcatCommunication.h"
#include <DynamixelShield.h>


GripperManager gripperManager;
EcatManager ecatManager;
DynamixelShield dxl;

 
void setup()
{
    #ifndef TESTING
    DEBUG_SERIAL.begin(DEBUG_BAUD);
    DEBUG_SERIAL.println("\n--------Starting---------");

    gripperManager = GripperManager();
    gripperManager.initialize(&dxl);
    ecatManager = EcatManager();  
    ecatManager.initialize();
    #else
    testHourglass();
    #endif
}

void loop()                                            
{        
    #ifndef TESTING                                             
    ecatManager.operate();
    EcatCommandInfo leftCommand = ecatManager.getEcatCommandInfoForGripper(Side::LEFT);
    EcatCommandInfo rightCommand = ecatManager.getEcatCommandInfoForGripper(Side::RIGHT);

    gripperManager.sendEcatCommandToGripper(leftCommand, Side::LEFT);
    gripperManager.sendEcatCommandToGripper(rightCommand, Side::RIGHT);
    gripperManager.operate();
    EcatReplyInfo leftReply = gripperManager.getEcatReplyInfoForGripper(Side::LEFT);
    EcatReplyInfo rightReply = gripperManager.getEcatReplyInfoForGripper(Side::RIGHT);
    
    ecatManager.setEcatReplyInfoForGripper(leftReply, Side::LEFT);
    ecatManager.setEcatReplyInfoForGripper(rightReply, Side::RIGHT);
    #endif
}
