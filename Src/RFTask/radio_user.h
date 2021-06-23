/*
 * ProcessRFState.h
 *
 *  Created on: 27. 5. 2019
 *      Author: jan.ropek
 */

#ifndef __RADIO_USER_H_
#define __RADIO_USER_H_

#include "gldef.h"
#include "TaskRF.h"
#include "radio.h"

//extern SPIConfiguration spiDevice;

#define FREQ_869_525_MHZ		869525000
#define FREQ_849_525_MHZ		829530000

typedef enum
{
	RF_CMD_NONE=0,
	RF_CMD_INIT_ON,
	RF_CMD_INIT_OFF,
	RF_INIT_START_RX,
	RF_INIT_START_TX,
	RF_CMD_SEND_REQUEST_INFO,
	RF_CMD_SEND_PAIRING_REQUEST,
	RF_CMD_SEND_PAIR_OK_TO_NODE,
	RF_CMD_SEND_NEGATE_STATE,
	RF_CMD_SEND_NEGATE_POWER,
	RF_CMD_SEND_UNIVERSAL_PAYLOAD_NOW


}RfCommands;


/* states of devices */
#define FENCEE_STATE_ON						1
#define FENCEE_STATE_OFF					0
#define FENCEE_POWER_HIGH					1
#define FENCEE_POWER_LOW					0


void RU_SX1262Assign(void);
void RU_GetTrueRandom32bit(uint32_t RandomData[], uint8_t CountOfRandomData);
void RU_SX1262Init(void);
void RU_RadioInit(void);
void RU_RadioDeinit(void);
uint8_t RU_IRQProcess(tDataState_Task_RF* GlobalData);
void RU_CommandProcess(RfCommands cmd,tDataState_Task_RF* GlobalData, DATA_QUEUE *ReceiveData);
void RU_RadioStandby(void);
void RU_RadioSleep(void);
void RU_LoRaConfigAndStartRX (uint8_t DR, bool Rx, uint32_t rxTimeout);
void RU_RFSetTXUp(void);
void RU_RFSetTXDown(void);
void RU_RFSwitch(Enum_RF_switch state);
uint16_t crc16(uint16_t crcValue, uint8_t newByte);

#endif /* RFTASK_PROCESSRFSTATE_H_ */
