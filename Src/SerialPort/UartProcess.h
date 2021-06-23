/*
 * UartProcess.h
 *
 *  Created on: 1. 2. 2021
 *      Author: jan.ropek
 */

#ifndef SERIALPORT_UARTPROCESS_H_
#define SERIALPORT_UARTPROCESS_H_

#include "main.h"

#define UART_BUFF_CRC_SIZE				(1)
#define UART_BUFF_HEADER_SIZE			(4)
#define MINIMAL_SIZE_USART_RX_MSG		(sizeof(SyncUartMsg)+UART_BUFF_HEADER_SIZE+1/*payload*/+UART_BUFF_CRC_SIZE/*crc*/)
#define MAXIMAL_SIZE_USART_RX_MSG		(sizeof(SyncUartMsg)+UART_BUFF_HEADER_SIZE+60/*payload*/+UART_BUFF_CRC_SIZE/*crc*/)
#define UART_CIRCLE_MAX_BUFFER_SIZE		(100)	//musi byt > 2= MAximal RX msg ?!?
#define MAX_SIZE_FOR_PAYLOAD			(40)
#define UART_CHECK_FREQUENCY			(50)	//(115200 = 14,4 B/ms => za 2ms = < 30 Byte)


/**
 *
 */
typedef enum
{
	eUART_MSG_EMPTY=0,     //!< eUART_MSG_EMPTY
	eUART_MSG_OK,          //!< eUART_MSG_OK
	eUART_MSG_TOO_SHORT,   //!< eUART_MSG_TOO_SHORT
	eUART_MSG_WRONG_HEADER,//!< eUART_MSG_WRONG_HEADER
	eUART_MSG_WRONG_CRC,   //!< eUART_MSG_WRONG_CRC
}eUARTBufferMasg;


/**
 *
 */
typedef union
{
	uint8_t		payload[UART_CIRCLE_MAX_BUFFER_SIZE];

}eUartMsgs;



extern uint8_t GlUartRxBugger[UART_CIRCLE_MAX_BUFFER_SIZE];

uint8_t 		UP_CalcCRC(uint8_t *data, uint8_t size);
bool 			UP_FindSyncWord(uint8_t *data, uint16_t sizeToSearch, uint16_t *headerStarts);
eUARTBufferMasg UP_FindAnyMsg(uint8_t **rxPacket,uint8_t *doCheckAgain);
void 			UP_UartSendData(uint8_t opCode, uint8_t *answer,uint8_t size);
void 			UP_UartTransmitRawData(uint8_t *buffer, uint8_t size);

#endif /* SERIALPORT_UARTPROCESS_H_ */
