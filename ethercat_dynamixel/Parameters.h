#ifndef __PARAMETERS__
#define __PARAMETERS__

// This is for debugging
#include <SoftwareSerial.h>
SoftwareSerial soft_serial(7, 8); // DYNAMIXELShield UART RX/TX
#define DEBUG_SERIAL soft_serial
const int DEBUG_BAUD = 19200; // 115200 doesn't work

const uint8_t LEFT_DXL_ID = 1;
const uint8_t RIGHT_DXL_ID = 2;

const float DXL_PROTOCOL_VERSION = 1.0;
const float DXL_BAUD_RATE = 57600;

const int ECAT_RESOLUTION = 255;

const int DEFAULT_LEFT_ZERO = 0;
const int DEFAULT_RIGHT_ZERO = 0;

const int RAW_SAFE_TORQUE = 104; // 0 to 1023
const int RAW_MAX_TORQUE = 800; // 0 to 1023
const int RAW_MAX_OPEN_POSITION = 2500;
const int RAW_TORQUE_NOISE_MAGNITUDE = 50;
const int MAX_TEMPERATURE = 45;

const int SAFE_TORQUE_EXCEEDED_MAX_TIME = 6.0; // in s
const int NONZERO_TORQUE_MAX_TIME = 120.0; // in s

#endif