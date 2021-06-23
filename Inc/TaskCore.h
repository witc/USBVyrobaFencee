/*
 * TaskCore.h
 *
 *  Created on: 2. 5. 2019
 *      Author: jan.ropek
 */

#ifndef TASKCORE_H_
#define TASKCORE_H_

#include "main.h"

#define INIT_FALSE				false
#define INIT_TRUE				true

void TaskCore(void const * argument);
void TaskCore(void const * argument);


typedef enum
{
	USB_CMD_NONE = 0,
	USB_CMD_HERE_I_AM = 1,
	USB_CMD_SEND_ADC_IN_MV = 2,
	USB_CMD_SEND_COEF = 3,
	USB_CMD_RF_SWITCH_SG = 6,
	USB_CMD_SWD_STATE = 8,
	USB_CMD_LED_BLINK = 9,
	USB_CMD_LED_ON = 10,
	USB_CMD_LED_OFF = 11,
	USB_CMD_READ_PIN_AUX6 = 12,

}eUSB_RF_CMDs;


typedef enum
{
	USB_CMD_SET_AUX1 = 1,
	USB_CMD_SET_AUX2 = 2,
	USB_CMD_SET_AUX3 = 3,
	USB_CMD_SET_AUX4 = 4,
	USB_CMD_READ_AUX7 = 5,
	USB_CMD_READ_AUX8 = 6,
	USB_CMD_BUTTON_PUSHED = 7,
	USB_CMD_SET_AUX_1_BLINK = 8,
	USB_CMD_SET_AUX_2_BLINK = 9,
	USB_CMD_SET_AUX_3_BLINK = 10,
	USB_CMD_SET_AUX_4_BLINK = 11,

}eUSB_Cmd;


/**
 *
 */
typedef struct{
	uint8_t MacHeader;
	uint8_t DevEui[8];
	uint8_t TxEui[8];
	uint8_t Zeros[7];
	uint8_t RndSesKEy[4];
	eUSB_RF_CMDs Cmd;
	uint8_t CoeafA[2];
	uint8_t CoefB[2];
	uint8_t RFU;
	uint8_t Crc[2];

}__packed USBLink_t;


typedef union
{
	USBLink_t Packet;
	uint8_t   rawData[sizeof(USBLink_t)];

}__packed USBRFLink_u;

#endif /* TASKCORE_H_ */
