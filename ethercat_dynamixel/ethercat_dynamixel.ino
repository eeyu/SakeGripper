// This is for debugging
#include <SoftwareSerial.h>
SoftwareSerial soft_serial(7, 8); // DYNAMIXELShield UART RX/TX
#define DEBUG_SERIAL soft_serial
const int DEBUG_BAUD = 9600; // 115200 doesn't work

#include <SPI.h>                    // SPI library
#include "Timer.h"

#define CUSTOM
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
    EcatCommandInfo command = ecatManager.getEcatCommandInfo();
    gripperManager.sendEcatCommand(command);
    gripperManager.operate();
    EcatReplyInfo reply = gripperManager.getEcatReplyInfo();
    ecatManager.setEcatReplyInfo(reply);
}
