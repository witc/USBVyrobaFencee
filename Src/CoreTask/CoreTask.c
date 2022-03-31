/*
 * CoreTask.c
 *
 *  Created on: 24. 9. 2019
 *      Author: jan.ropek
 */

#include "main.h"
#include "gldef.h"
#include "TaskCore.h"
#include "radio_user.h"
#include "stdio.h"
#include "string.h"
#include "UartProcess.h"

uint8_t		GlBlinkingMask=0;



extern UART_HandleTypeDef huart1;
extern volatile osMessageQId QueueRFHandle;;
extern osMessageQId QueueCoreHandle;
extern osTimerId TimerLEDHandle;
extern uint8_t	syncWord[5];

#define USB_RF_LINK_SIZE	36
uint8_t GlUartRx[USB_RF_LINK_SIZE];

uint16_t GlUsartIndex;
#define CIRCLE_BUFF_SIZE	100
uint8_t GlCircleBuffer[CIRCLE_BUFF_SIZE];
uint8_t GlUartRxBugger[UART_CIRCLE_MAX_BUFFER_SIZE];

void SendUartMsg(uint8_t opCode, uint8_t value, uint8_t size);

volatile uint8_t LEDColorBlink=0;
extern osTimerId TimerSystemLEDHandle;

void CallbackButtonCheck(void const * argument)
{
	DATA_QUEUE	SendData;
	DATA_QUEUE	SendData2;
	SendData.pointer = NULL;
	SendData2.pointer = NULL;

	uint32_t pin = LL_GPIO_ReadInputPort(AUX_6_GPIO_Port);
	if((pin & AUX_6_Pin)==0)
	{
		/* stisknuto */
		SendData.Address = ADDR_CORE_BUTTON_PUSHED;
		SendData.Data = AUX_6_Pin;
		xQueueSend(QueueCoreHandle, &SendData, portMAX_DELAY);

	}

	if((pin & AUX_5_Pin)==0)
	{
		/* stisknuto */
		SendData2.Address = ADDR_CORE_BUTTON_PUSHED;
		SendData2.Data = AUX_5_Pin;
		xQueueSend(QueueCoreHandle, &SendData2, portMAX_DELAY);

	}

}

void CallbackSystemLED(void const * argument)
{
	LL_GPIO_ResetOutputPin(LED_GREEN_GPIO_Port,LED_GREEN_Pin);
}

/**
 *
 * @param htim
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

	DATA_QUEUE SendData;
	SendData.pointer = NULL;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	SendData.Address = ADDR_TO_CORE_UART_READ_RX_BUFFER;
	xQueueSendFromISR(QueueCoreHandle,&SendData,&xHigherPriorityTaskWoken);

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 *
 */
void Callback_LED(void const * argument)
{
	if(GlBlinkingMask&(1<<0))
	{
		LL_GPIO_TogglePin(AUX_1_GPIO_Port,AUX_1_Pin);
	}

	if(GlBlinkingMask&(1<<1))
	{
		LL_GPIO_TogglePin(AUX_2_GPIO_Port,AUX_2_Pin);
	}

	if(GlBlinkingMask&(1<<2))
	{
		LL_GPIO_TogglePin(AUX_3_GPIO_Port,AUX_3_Pin);
	}


	if(GlBlinkingMask&(1<<3))
	{
		LL_GPIO_TogglePin(AUX_4_GPIO_Port,AUX_4_Pin);
	}

}



void Decode_Uart_Rx(uint8_t *rxBuffer)
{
	uint8_t pinState;
	uint8_t Txbuffer[10];
	eUSB_Cmd cmd = rxBuffer[0];

	switch (cmd)
	{
		case USB_CMD_SET_AUX1:
			GlBlinkingMask &=~ (1<<0);
			HAL_GPIO_WritePin(AUX_1_GPIO_Port,AUX_1_Pin, rxBuffer[1] == 0 ? false : true);

			break;

		case USB_CMD_SET_AUX2:
			GlBlinkingMask &=~ (1<<1);
			HAL_GPIO_WritePin(AUX_2_GPIO_Port,AUX_2_Pin, rxBuffer[1] == 0 ? false : true);

			break;

		case USB_CMD_SET_AUX3:
			GlBlinkingMask &=~ (1<<2);
			HAL_GPIO_WritePin(AUX_3_GPIO_Port,AUX_3_Pin, rxBuffer[1] == 0 ? false : true);

			break;

		case USB_CMD_SET_AUX4:
			GlBlinkingMask &=~ (1<<3);
			HAL_GPIO_WritePin(AUX_4_GPIO_Port,AUX_4_Pin, rxBuffer[1] == 0 ? false : true);

			break;

		case USB_CMD_READ_AUX7:
			pinState = HAL_GPIO_ReadPin(AUX_7_GPIO_Port,AUX_7_Pin);
			SendUartMsg(7,pinState,1);
			break;

		case USB_CMD_READ_AUX8:
			pinState = HAL_GPIO_ReadPin(AUX_8_GPIO_Port,AUX_8_Pin);
			SendUartMsg(8,pinState,1);
//			Txbuffer
			break;

		case USB_CMD_SET_AUX_1_BLINK:
			HAL_GPIO_WritePin(AUX_1_GPIO_Port,AUX_1_Pin,  false);
			GlBlinkingMask |= (1<<0);

			break;

		case USB_CMD_SET_AUX_2_BLINK:
			HAL_GPIO_WritePin(AUX_2_GPIO_Port,AUX_2_Pin,  false);
			GlBlinkingMask |= (1<<1);

			break;
		case USB_CMD_SET_AUX_3_BLINK:
			HAL_GPIO_WritePin(AUX_3_GPIO_Port,AUX_3_Pin,  false);
			GlBlinkingMask |= (1<<2);

			break;
		case USB_CMD_SET_AUX_4_BLINK:
			HAL_GPIO_WritePin(AUX_4_GPIO_Port,AUX_4_Pin,  false);
			GlBlinkingMask |= (1<<3);

			break;

		default:
			break;
	}
}


void SendUartMsg(uint8_t opCode, uint8_t value, uint8_t size)
{
	uint8_t buffer[100];
	uint8_t sumSize = 0;
	memset(buffer,0,sizeof(buffer));

	/*syn word */
	buffer[0] = 0x2d;
	buffer[1] = 0xd4;
	sumSize+=2;
	/* header */
	buffer[2] = size;
	buffer[3] = size;
	buffer[4] = size;
	buffer[5] = UP_CalcCRC(&buffer[2],3);
	sumSize+=4;
	/*payload */
	buffer[6] = opCode;
	buffer[7] = value;
	sumSize+=2;

	/* crc payload */
	buffer[8]  = UP_CalcCRC(&buffer[0], sumSize);
	sumSize+=1;

	HAL_UART_Transmit(&huart1,buffer,sumSize,100000);
	//UP_UartTransmitRawData(UartTxDMABuffer,actualSize);
}



void InitUart()
{
	/*Uart  Init DMA */
	LL_DMA_DisableChannel(DMA1,LL_DMA_CHANNEL_3);
	LL_DMA_ConfigTransfer(DMA1, LL_DMA_CHANNEL_3,
							LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
							LL_DMA_PRIORITY_LOW               |
							LL_DMA_MODE_CIRCULAR              |
							LL_DMA_PERIPH_NOINCREMENT         |
							LL_DMA_MEMORY_INCREMENT           |
							LL_DMA_PDATAALIGN_BYTE            |
							LL_DMA_MDATAALIGN_BYTE);

	LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_3,
							 LL_USART_DMA_GetRegAddr(USART1, LL_USART_DMA_REG_DATA_RECEIVE),
							 (uint32_t)GlUartRxBugger,LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3));

	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, UART_CIRCLE_MAX_BUFFER_SIZE);
	LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_3, LL_DMA_REQUEST_3);

	/* Enable DMA RX Interrupt */
	LL_USART_EnableDMAReq_RX(USART1);
	/* Enable DMA Channel Rx */
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);

//	osTimerStart(TimerUartRxCheckHandle,TIME_TO_CHECK_UART_RX_BUFFER);
	HAL_NVIC_DisableIRQ(TIM6_IRQn);
	HAL_NVIC_ClearPendingIRQ(TIM6_IRQn);
	HAL_NVIC_EnableIRQ(TIM6_IRQn);

	LL_TIM_SetAutoReload(TIM6,__LL_TIM_CALC_ARR(4194000,LL_TIM_GetPrescaler(TIM6),UART_CHECK_FREQUENCY));   //(1/MINIMAL_SIZE_USART_RX_MSG)
	LL_TIM_EnableIT_UPDATE(TIM6);
	LL_TIM_SetCounter(TIM6,0);
	LL_TIM_EnableCounter(TIM6);

}


void TaskCore(void const * argument)
{
   static DATA_QUEUE SendData;
   static DATA_QUEUE ReceiveData;
   SendData.pointer=NULL;
   uint8_t			*RxUartMsg = NULL;
   uint8_t			doCheckAgain = 0;
   uint8_t cmd = 2;
   uint8_t txBuff[]={0x1,0x2,0,0};	// posledni 2 byte jsou pro crc
   uint8_t aux6 = 0;
   unsigned int crc=0xffff;
   BaseType_t ReturnValue;

   SendData.Data=DATA_STATE_DEVICE_INIT_ON;
   SendData.Address=ADDR_CORE_STATE_DEVICE;
   xQueueSend(QueueRFHandle,&SendData,portMAX_DELAY);

   InitUart();
   osTimerStart(TimerLEDHandle,500);

   for(uint8_t led=0;led<6;led++)
   {
   	  LL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
   	  osDelay(100);
   }

   for(;;)
   {
	   ReturnValue = xQueueReceive(QueueCoreHandle, &ReceiveData, portMAX_DELAY);

	   if (ReturnValue == pdPASS)
	   {
		   switch (ReceiveData.Address)
		   {
			   case ADDR_TO_CORE_UART_READ_RX_BUFFER:
					/* periodic scanning circular uart buffer*/
					do
					{
						if(UP_FindAnyMsg(&RxUartMsg,&doCheckAgain)==eUART_MSG_OK)
						{
							LL_GPIO_SetOutputPin(LED_GREEN_GPIO_Port,LED_GREEN_Pin);
							//PCT_DecodeUartRxMsg(RxUartMsg);
							Decode_Uart_Rx(RxUartMsg);
							vPortFree(RxUartMsg);
							RxUartMsg = NULL;
							osTimerStart(TimerSystemLEDHandle,50);
							//LL_GPIO_ResetOutputPin(LED_GREEN_GPIO_Port,LED_GREEN_Pin);
						}
					}
					while(doCheckAgain == 1 );

					LL_TIM_EnableIT_UPDATE(TIM6);
					LL_TIM_SetCounter(TIM6,0);
					LL_TIM_EnableCounter(TIM6);
					break;

			case ADDR_CORE_BUTTON_PUSHED:

				if(ReceiveData.Data == AUX_5_Pin)
				{
					SendUartMsg(7,5,2);
				}
				else if(ReceiveData.Data == AUX_6_Pin)
				{
					SendUartMsg(7,6,2);
				}

				break;

			default:
				break;
		}
	   }
   }



//   for(;;)
//   {
//	  temp =rand();      // Returns a pseudo-random integer between 0 and RAND_MAX.
//	  randNum= temp%256;
//	  HAL_UART_Transmit(&huart1,&randNum,1,1000);
//	  osDelay(10);
//
//   }

}
