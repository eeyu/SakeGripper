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
#define CUST_BYTE_NUM_IN	4
#define TOT_BYTE_NUM_ROUND_OUT	8
#define TOT_BYTE_NUM_ROUND_IN	4


typedef union												//---- output buffer ----
{
	uint8_t  Byte [TOT_BYTE_NUM_ROUND_OUT];
	struct
	{
		uint8_t     g1Command;
		uint8_t     g1Position;
		uint8_t     g1Torque;
		uint8_t     g2Command;
		uint8_t     g2Position;
		uint8_t     g2Torque;
	}Cust;
} PROCBUFFER_OUT;


typedef union												//---- input buffer ----
{
	uint8_t  Byte [TOT_BYTE_NUM_ROUND_IN];
	struct
	{
		uint8_t     g1Busy;
		uint8_t     g1Position;
		uint8_t     g2Busy;
		uint8_t     g2Position;
	}Cust;
} PROCBUFFER_IN;

#endif