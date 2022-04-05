// This is for debugging
#include <SoftwareSerial.h>
SoftwareSerial soft_serial(7, 8); // DYNAMIXELShield UART RX/TX
#define DEBUG_SERIAL soft_serial
const float DEBUG_BAUD = 115200;

#include <SPI.h>                    // SPI library
#include <DynamixelShield.h>
#include "Timer.h"
#include "LowLevelGripper.h"

#define CUSTOM
#include "SakeGripperEcat.h" // this is the project name...find the rest of the files in the easycat configurator folder
#include "EasyCAT.h"                // EasyCAT library to interface the LAN9252
#include "EcatCommunication.h"

//---- easycat stuff ---------------------------------
EasyCAT EASYCAT;            

Timer ecat_timer;
EcatCommandSignal last_signal = WAITING;
bool newCommandSignalRecieved = false;

//---- dynamixel declaration ------------------------------------------------------------------------------
const uint8_t DXL_ID = 1;
const float DXL_PROTOCOL_VERSION = 1.0;
const float DXL_BAUD_RATE = 57142;
DynamixelShield dxl;
Gripper gripper;

EcatCommandInfo ecat_command_info;
EcatReplyInfo ecat_reply_info;


Timer dxl_timer;

//---- setup ---------------------------------------------------------------------------------------
 
void setup()
{
  beginEasyCat();
  beginDynamixel();

  ecat_timer.reset(0.001);
  dxl_timer.reset(0.01);
}

void beginEasyCat() {
    DEBUG_SERIAL.print ("\nEasyCAT - Generic EtherCAT slave\n");          // print the banner
                                                                    //---- initialize the EasyCAT board -----                                       
    if (EASYCAT.Init() == true){                                                               
      DEBUG_SERIAL.print ("initialized");                                
    } else {                                                               
      DEBUG_SERIAL.print ("initialization failed");                        
    } 

    ecat_timer.usePrecision();
}

void beginDynamixel() {
    DEBUG_SERIAL.begin(DEBUG_BAUD);
  
    dxl.begin(DXL_BAUD_RATE);
    dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

    gripper = Gripper(DXL_ID, &dxl);
}


//---- main loop ----------------------------------------------------------------------------------------
 
void loop()                                             // In the main loop we must call ciclically the 
{                                                       // EasyCAT task and our application
  doEcatTask();
  doGripperTask();
}

void doEcatTask() {
  EASYCAT.MainTask();
  if (ecat_timer.timeOut())                    
  {                                                     
    ecat_timer.reset();

    ecat_command_info = EcatCommandInfo(EASYCAT.BufferOut.Cust.signal,
                                        EASYCAT.BufferOut.Cust.position,
                                        EASYCAT.BufferOut.Cust.torque);

    EASYCAT.BufferIn.Cust.gripper_busy = ecat_reply_info.gripper_busy;
  }
}

void doGripperTask() {
  if (dxl_timer.timeOut()) {
    gripper.operate();
    updateNewCommandRecieved();
    if (!gripper.isBusy() && newCommandSignalRecieved) {
      executeDynamixelCommand();
    }
  }
  ecat_reply_info.gripper_busy = gripper.isBusy();
}

void executeDynamixelCommand() {
  switch (ecat_command_info.signal) {
    case CALIBRATE:
      // Calibrate is the only blocking action, so need to send busy signal manually
      gripper.setBusy();
      EASYCAT.BufferIn.Cust.gripper_busy = true;
      EASYCAT.MainTask();

      DEBUG_SERIAL.println("execute calibrate");
      gripper.calibrate();
      break;
    case GOTO:
      gripper.gotoPositionWithTorque(ecat_command_info.position, ecat_command_info.torque);
      DEBUG_SERIAL.println("execute goto");
      break;
    case RELEASE:
      gripper.release();
      DEBUG_SERIAL.println("execute release");
      break;
    case OPEN:
      gripper.open();
      DEBUG_SERIAL.println("execute open");
      break;
    default:
      break;
  }
}



void updateNewCommandRecieved() {
  EcatCommandSignal current_signal = ecat_command_info.signal;
  newCommandSignalRecieved = (last_signal == WAITING) && (current_signal != WAITING);
  last_signal = current_signal;
}
