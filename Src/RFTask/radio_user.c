/*
 * radio_user.c
 *
 *  Created on: 2. 11. 2019
 *      Author: jan.ropek
 */


/*
 * ProcessRFState.c
 *
 *  Created on: 27. 5. 2019
 *      Author: jan.ropek
 */

#include "TaskRF.h"
#include "radio_general.h"
#include "radio_user.h"
#include "LoRa_Codec.h"
#include "main.h"
#include "sx126x.h"
#include "radio.h"
#include "gldef.h"


extern volatile osMessageQId QueueCoreHandle;
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart1;
/* Definice */
SPIConfiguration spiDevice2;
SPIConfiguration spiDevice;

uint8_t	syncWord[5]={0xAA,0xBB,0xCC,0xDD,'\n'};
/**
 *
 */
void RU_SX1262Assign(void)
{
	taskENTER_CRITICAL();

	spiDevice2.AtomicActionEnter=vPortEnterCritical;
	spiDevice2.AtomicActionExit=vPortExitCritical;
	spiDevice2.pin_BUSY.port=SX1262_BUSY_GPIO_Port;
	spiDevice2.pin_BUSY.pin=SX1262_BUSY_Pin;
	spiDevice2.pin_RESET.port=SX1262_RESET_GPIO_Port;
	spiDevice2.pin_RESET.pin=SX1262_RESET_Pin;
	spiDevice2.pin_DIO1.port=SX1262_DIO1_GPIO_Port;
	spiDevice2.pin_DIO1.pin=SX1262_DIO1_Pin;
	spiDevice2.pin_NSS.port=SX1262_NSS_GPIO_Port;
	spiDevice2.pin_NSS.pin=SX1262_NSS_Pin;
	spiDevice2.pin_RF_SWITCH.port=SX1262_RF_SWITCH_GPIO_Port;
	spiDevice2.pin_RF_SWITCH.pin=SX1262_RF_SWITCH_Pin;
	spiDevice2.RadioRFSwitch=RU_RFSwitch;
#if (RF_USE_DMA==1)

#endif
	spiDevice2.target=&hspi1;

	taskEXIT_CRITICAL();
	/* Select to high*/
	LL_GPIO_SetOutputPin(spiDevice2.pin_NSS.port,spiDevice2.pin_NSS.pin);
	LL_GPIO_SetOutputPin(spiDevice2.pin_RF_SWITCH.port,spiDevice2.pin_RF_SWITCH.pin);

	RadioInit();

	/* zkouska SPI komunikace*/
	if(RG_SX126xReadRegister(0x08AC)!=0x94 )
	{
		while(1);//SendErrorCode_AND_Reset(89,63);
	}
}

/*
 * @brief : Serve IRQ from Semtech
 */
uint8_t RU_IRQProcess(tDataState_Task_RF* GlobalData)
{
	DATA_QUEUE SendData;
	SendData.pointer=NULL;
	PacketStatus_t RadioPktStatus;
	uint16_t irqRegs =0;
	uint8_t size=0;
	uint8_t RxBuffer[PACKET_MAX_SIZE];

#if (TRC_USE_TRACEALYZER_RECORDER == 1)
	vTracePrint(myChannel,"Prijato IRQ z SX1262");
#endif

	RadioStandby();
	irqRegs = SX126xGetIrqStatus();
	SX126xClearIrqStatus(IRQ_RADIO_ALL);

	switch (GlobalData->RF_State)
	{
	    case RF_RX_RUNNING:

            if (((irqRegs & IRQ_RX_DONE) == IRQ_RX_DONE)&& ((irqRegs & IRQ_CRC_ERROR) != IRQ_CRC_ERROR)) //Data recieved
            {
                if (SX126xGetPayload(RxBuffer, &size, PACKET_MAX_SIZE-2)== 0)
                {
                  // 	LL_GPIO_SetOutputPin(LED2_GPIO_Port,LED2_Pin);
                	/*zvetseni size o crc*/
                	size+=2;

                    SX126xGetPacketStatus(&RadioPktStatus);	//RSSI, SNR, Freq Error,..
                    HAL_UART_Transmit(&huart1,syncWord,sizeof(syncWord),10000);
                    uint8_t cmd = 0x1;
                    HAL_UART_Transmit(&huart1,&cmd,1,10000);

                    HAL_UART_Transmit(&huart1,&size ,1,10000);

                    unsigned int crc=0xffff;
					for(uint8_t i=0;i<(size-2);i++)
					{
						crc = crc16(crc,RxBuffer[i]);
					}

					RxBuffer[size-2] = (uint8_t)(crc&0xFF);
					RxBuffer[size-1]=(uint8_t)(crc>>8);
                    HAL_UART_Transmit(&huart1,RxBuffer,size,10000);
                   // osDelay(100);
                   // LL_GPIO_ResetOutputPin(LED2_GPIO_Port,LED2_Pin);
                 }
		    }

	    	RadioStandby();
	    	RU_LoRaConfigAndStartRX(0,true,10000);
		    break;

        case RF_TX_RUNNING:
            RU_LoRaConfigAndStartRX(0,true,10000);
        	LL_GPIO_SetOutputPin(LED1_GPIO_Port,LED1_Pin);
        	osDelay(100);
        	LL_GPIO_ResetOutputPin(LED1_GPIO_Port,LED1_Pin);
        //	RadioSend(NULL,10);

            break;

	    default:
		    break;
	}


	return 0;

}

/*
 * @brief: Command recieved to RF task
 */
void RU_CommandProcess(RfCommands cmd,tDataState_Task_RF* GlobalData, DATA_QUEUE *ReceiveData)
{
	uint8_t				*TxPacket;

	switch (cmd)
	{
		case DATA_CMD_START_RX:
			if (GlobalData->RF_State != RF_IDLE)
			{
				RadioStandby();
				(void) SX126xGetIrqStatus();
				SX126xClearIrqStatus(IRQ_RADIO_ALL);
			}

			RU_LoRaConfigAndStartRX(0,true,1000);

			break;

		case RF_CMD_INIT_OFF:
			RadioStandby();
			(void) SX126xGetIrqStatus();
			SX126xClearIrqStatus(IRQ_RADIO_ALL);
			RadioSleep();
			osThreadYield();
			break;

		case RF_CMD_INIT_ON:

			RadioInit();
			RadioStandby();
			/* Set RX window like RX Window in CLASS C in LRWAN*/
			RU_LoRaConfigAndStartRX(0, true, RX_TIMEOUT_LONG);
			break;

		case RF_CMD_SEND_REQUEST_INFO:


		//	RU_RFSetTXDown();
		//	LD_LoRaDownRequestInfo(RfPacket->DR, &RfPacket->SpecificAesKey,&RfPacket->DEV_EUI);

			break;

		case RF_CMD_SEND_UNIVERSAL_PAYLOAD_NOW:

			TxPacket=ReceiveData->pointer;
			SX126xClearIrqStatus(IRQ_RADIO_ALL);
			RadioStandby();

			RU_RFSetTXUp();
			taskENTER_CRITICAL();
			RadioSend(TxPacket,ReceiveData->temp);
			taskEXIT_CRITICAL();

			break;

		default:
			break;
	}
}




/*
 * brief: Set RX config
 */
void RU_LoRaConfigAndStartRX (uint8_t DR, bool Rx, uint32_t rxTimeout)
{

  switch (DR)
  {
    case 0:	//Dwon
    	RadioSetRxConfig (FREQ_869_525_MHZ, MODEM_LORA /*mod*/, 7 /*BW*/,
    				6 /*SF*/, 1 /*cr*/, 0 /* BW AFC*/, 8 /*preamble*/,
    				0 /*symbtimeout to lock*/, false /*fixlen*/,
    				8 /* payloadlen*/,
    				true/*crc on*/,
    				0 /*frhop*/, 0 /* hoperiod*/, false /* iq inverted*/,
    				1 /*Continous*/);

       break;

    case 1:	//UP
        RadioSetRxConfig (FREQ_869_525_MHZ, MODEM_LORA /*mod*/, 7 /*BW*/,
			6 /*SF*/, 1 /*cr*/, 0 /* BW AFC*/, 8 /*preamble*/,
			0 /*symbtimeout to lock*/, false /*fixlen*/,
			8 /* payloadlen*/,
			true/*crc on*/,
			0 /*frhop*/, 0 /* hoperiod*/, false /* iq inverted*/,
			1 /*Continous*/);
      //rxTimeout=10000;
      break;

    default:
      break;
    }

  if (Rx == true)   RadioRxBoosted (rxTimeout);
}


/*
 * brief: set default parametter for standard RX window2 - Dwon Packets
 */
void RU_RFSetTXDown(void)
{
	int8_t 	 TxPower=22;
	RadioSetTxConfig(FREQ_869_525_MHZ, MODEM_LORA /* modem*/,(int8_t)TxPower /*power*/, 0/*fdev*/,  7/*BW*/,9/*SF*/,
								 1 /*coderate*/, 8 /*preamble*/, false /*fixlen*/, true/*crc on*/,
								 0 /*freqhop*/, 0 /* hopeperiod*/, false /* IQ inverted*/, 0xff /*Timeout*/);
}


/*
 * brief: set default parametter for UP packets
 */
void RU_RFSetTXUp(void)
{
	int8_t 	 TxPower=22;

	RadioSetTxConfig(FREQ_869_525_MHZ, MODEM_LORA /* modem*/,(int8_t)TxPower /*power*/, 0/*fdev*/,  7/*BW*/,6/*SF*/,
					 	 	 	 1 /*coderate*/, 8 /*preamble*/, false /*fixlen*/, true/*crc on*/,
								 0 /*freqhop*/, 0 /* hopeperiod*/, false /* IQ inverted*/, 0xff /*Timeout*/);
}


void RU_RFSwitch(Enum_RF_switch state)
{
	switch (state)
	{
		case SWITCH_RX:
			HAL_GPIO_WritePin(spiDevice2.pin_RF_SWITCH.port,spiDevice2.pin_RF_SWITCH.pin,1);
			break;

		case SWITCH_TX:
			HAL_GPIO_WritePin(spiDevice2.pin_RF_SWITCH.port,spiDevice2.pin_RF_SWITCH.pin,0);

		default:
			break;
	}
}


/**
 *brief: Calculate  crc - 16/MODBUS - init 0xffff
 */
uint16_t crc16(uint16_t crcValue, uint8_t newByte)
{
	int i;

	crcValue ^= (uint16_t)newByte;
	for (i = 0; i < 8; ++i) {
		if (crcValue & 1)
			crcValue = (crcValue >> 1) ^ 0xA001;
		else
			crcValue = (crcValue >> 1);
	}

	return crcValue;
}



