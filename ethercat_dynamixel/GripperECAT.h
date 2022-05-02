#ifndef CUSTOM_PDO_NAME_H
#define CUSTOM_PDO_NAME_H

//-------------------------------------------------------------------//
//                                                                   //
//     This file has been created by the Easy Configurator tool      //
//                                                                   //
//     Easy Configurator project GripperECAT.prj
//                                                                   //
//-------------------------------------------------------------------//


#define CUST_BYTE_NUM_OUT	6
#define CUST_BYTE_NUM_IN	10
#define TOT_BYTE_NUM_ROUND_OUT	8
#define TOT_BYTE_NUM_ROUND_IN	12


typedef union												//---- output buffer ----
{
	uint8_t  Byte [TOT_BYTE_NUM_ROUND_OUT];
	struct
	{
		uint8_t     leftCommand;
		uint8_t     leftPosition;
		uint8_t     leftTorque;
		uint8_t     rightCommand;
		uint8_t     rightPosition;
		uint8_t     rightTorque;
	}Cust;
} PROCBUFFER_OUT;


typedef union												//---- input buffer ----
{
	uint8_t  Byte [TOT_BYTE_NUM_ROUND_IN];
	struct
	{
		uint8_t     leftBusy;
		uint8_t     leftPosition;
		uint8_t     leftTorque;
		uint8_t     leftTemperature;
		uint8_t     leftError;
		uint8_t     rightBusy;
		uint8_t     rightPosition;
		uint8_t     rightTorque;
		uint8_t     rightTemperature;
		uint8_t     rightError;
	}Cust;
} PROCBUFFER_IN;

#endif