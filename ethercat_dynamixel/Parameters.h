#ifndef __PARAMETERS__
#define __PARAMETERS__

// This is for debugging
#include <SoftwareSerial.h>
SoftwareSerial soft_serial(7, 8); // DYNAMIXELShield UART RX/TX
#define DEBUG_SERIAL soft_serial
const int DEBUG_BAUD = 9600; // 115200 doesn't work

const uint8_t GRIPPER1_DXL_ID = 1;
const uint8_t GRIPPER2_DXL_ID = 2;
// const float DXL_PROTOCOL_VERSION = 1.0;
// const float DXL_BAUD_RATE = 57142;
const float DXL_PROTOCOL_VERSION = 1.0;
const float DXL_BAUD_RATE = 57600;

#define GRIPPER_RESOLUTION_MIN 0
#define GRIPPER_RESOLUTION_MAX 255

const int DEFAULT_GRIPPER1_ZERO = 0;
const int DEFAULT_GRIPPER2_ZERO = 1326;

#endif