#ifndef __PARAMETERS__
#define __PARAMETERS__

const uint8_t GRIPPER1_DXL_ID = 1;
const uint8_t GRIPPER2_DXL_ID = 2;
// const float DXL_PROTOCOL_VERSION = 1.0;
// const float DXL_BAUD_RATE = 57142;
const float DXL_PROTOCOL_VERSION = 1.0;
const float DXL_BAUD_RATE = 57600;

#define GRIPPER_RESOLUTION_MIN 0
#define ECAT_RESOLUTION 255

const int DEFAULT_GRIPPER1_ZERO = 0;
const int DEFAULT_GRIPPER2_ZERO = 1326;

const int RAW_SAFE_TORQUE = 104; // 0 to 1023
const int RAW_MAX_TORQUE = 800; // 0 to 1023
const int RAW_MAX_OPEN_POSITION = 2500;
const int RAW_TORQUE_NOISE_MAGNITUDE = 24;

const int SAFE_TORQUE_EXCEEDED_MAX_TIME = 6.0; // in s
const int NONZERO_TORQUE_MAX_TIME = 120.0; // in s

#endif