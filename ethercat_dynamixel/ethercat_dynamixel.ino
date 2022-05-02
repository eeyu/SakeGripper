// This is for debugging
#include <SoftwareSerial.h>
SoftwareSerial soft_serial(7, 8); // DYNAMIXELShield UART RX/TX
#define DEBUG_SERIAL soft_serial
const int DEBUG_BAUD = 9600; // 115200 doesn't work

#include <SPI.h>                    // SPI library
#include "Timer.h"

#define CUSTOM
#include "Side.h"
#include "Parameters.h"
#include "GripperManager.h"
#include "EcatManager.h"
#include "EcatCommunication.h"

GripperManager gripperManager;
EcatManager ecatManager;
 
void setup()
{
    DEBUG_SERIAL.begin(DEBUG_BAUD);
    DEBUG_SERIAL.println("starting");
    gripperManager = GripperManager();
    gripperManager.initialize();
    ecatManager = EcatManager();  
    ecatManager.initialize();
}

void loop()                                            
{                                                     
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
}
