/*
 * TaskRF.h
 *
 *  Created on: 21. 2. 2018
 *      Author: jirik
 */

#ifndef TASKRF_H_
#define TASKRF_H_
#include "radio.h"
/************************************************************************/
/* Area #DEFINE															*/
/************************************************************************/
#define PACKET_MAX_SIZE			50
#define RX_TIMEOUT_LONG			1000
typedef struct
{
	uint8_t Temp;
	RadioState_t RF_State;
} tDataState_Task_RF;


/************************************************************************/
/* MAKRO with parameters                                                */
/************************************************************************/

/************************************************************************/
/* Area TYPEDEF                                                         */
/************************************************************************/

/************************************************************************/
/* Declaration global variables - EXTERN                                */
/************************************************************************/


/************************************************************************/
/* Declaration global function											*/
/************************************************************************/
void TaskRF(void const * argument);
void LoRaConfigAndStartRX(uint8_t DR, bool Rx, uint32_t rxTimeout);

#endif /* TASKRF_H_ */
