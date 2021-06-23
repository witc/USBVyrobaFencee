/*
 * TaskRF.c
 *
 *  Created on: 21. 2. 2018
 *      Author: jirik
 */

/************************************************************************/
/* include header												   		*/
/************************************************************************/

#include "main.h"
#include "gldef.h"
#include "TaskCore.h"
#include "TaskRF.h"
#include "radio.h"
#include "radio_general.h"
#include "radio_user.h"


/************************************************************************/
/* Declaration importing objects                                        */
/************************************************************************/
extern volatile osMessageQId QueueCoreHandle;
extern volatile osMessageQId QueueRFHandle;
extern volatile osThreadId TaskRFHandle;

extern UART_HandleTypeDef huart1;
/************************************************************************/
/* Definition global variables                                          */
/************************************************************************/

#if (TRC_USE_TRACEALYZER_RECORDER == 1)
	volatile traceString myChannel;
#endif

/************************************************************************/
/* Local #DEFINE														*/
/************************************************************************/
#define TERMINATE_MEASURE_BATT		10
/************************************************************************/
/* Local TYPEDEF												  	 	*/
/************************************************************************/
typedef enum
{
	STATE_INIT,
	STATE_OFF,
	STATE_START_ON,
	STATE_ON,
	STATE_START_OFF
} ENUM_STATE_CODES_RF;


typedef struct
{
	ENUM_STATE_CODES_RF	ActualState;
	ENUM_STATE_CODES_RF	PreviousState;
} STRUCT_STATE_AUTOMAT_RF;


/************************************************************************/
/* Definition local variables										   */
/************************************************************************/

/************************************************************************/
/* Declaration functions											   */
/************************************************************************/


static uint8_t RF_StateINIT(DATA_QUEUE ReceiveData,
		tDataState_Task_RF* GlobalData, STRUCT_STATE_AUTOMAT_RF* StateAutomat,
		void** PointerMalloc);

static uint8_t RF_StateOFF(DATA_QUEUE ReceiveData,
		tDataState_Task_RF* GlobalData, STRUCT_STATE_AUTOMAT_RF* StateAutomat,
		void** PointerMalloc);

static uint8_t RF_StateStartON(DATA_QUEUE ReceiveData,
		tDataState_Task_RF* GlobalData, STRUCT_STATE_AUTOMAT_RF* StateAutomat,
		void** PointerMalloc);

static uint8_t RF_StateON(DATA_QUEUE ReceiveData,
		tDataState_Task_RF* GlobalData, STRUCT_STATE_AUTOMAT_RF* StateAutomat,
		void** PointerMalloc);

static uint8_t RF_StateStartOFF(DATA_QUEUE ReceiveData,
		tDataState_Task_RF* GlobalData, STRUCT_STATE_AUTOMAT_RF* StateAutomat,
		void** PointerMalloc);

static uint8_t (*StateRF[])(DATA_QUEUE, tDataState_Task_RF*,
		STRUCT_STATE_AUTOMAT_RF*, void**) =
		{	RF_StateINIT, RF_StateOFF, RF_StateStartON , RF_StateON, RF_StateStartOFF
};


static void WatchdogRefreshOK_RF(void);
/************************************************************************/
/* Definition functions                                                 */
/************************************************************************/




/**
 *
 * @param ReceiveData
 * @param GlobalData
 * @param StateAutomat
 * @param PointerMalloc
 * @return
 */
static uint8_t RF_StateINIT(DATA_QUEUE ReceiveData,
		tDataState_Task_RF* GlobalData, STRUCT_STATE_AUTOMAT_RF* StateAutomat,
		void** PointerMalloc)
{

	return 0;
}

/**
 *
 * @param ReceiveData
 * @param GlobalData
 * @param StateAutomat
 * @param PointerMalloc
 * @return
 */
static uint8_t RF_StateOFF (DATA_QUEUE ReceiveData, tDataState_Task_RF* GlobalData,
	     STRUCT_STATE_AUTOMAT_RF* StateAutomat, void** PointerMalloc)
{
    DATA_QUEUE SendData;
    SendData.pointer = NULL;
    uint32_t random;
    uint8_t randNum;

	switch (ReceiveData.Address)
	{
	case ADDR_CORE_STATE_DEVICE:
		if ((ReceiveData.Data == DATA_STATE_DEVICE_INIT_ON)	|| (ReceiveData.Data == DATA_STATE_DEVICE_ON))
		{
			RU_CommandProcess(DATA_CMD_RF_INIT_ON,GlobalData,&ReceiveData);

			/*Enable IRQ semtech */
			LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_2);
			LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_2);

			RU_LoRaConfigAndStartRX(0,true,RX_TIMEOUT_LONG);

			/*Send answer back to Core*/
			SendData.Address = ADDR_CORE_RF_STATE_DONE;
			SendData.Data = DATA_STATE_ON_DONE;
			xQueueSend(QueueCoreHandle, &SendData, portMAX_DELAY);

			StateAutomat->PreviousState = StateAutomat->ActualState;
			StateAutomat->ActualState = STATE_ON;

			//RU_RFSetTXDown();
//			RadioSend(NULL,10);
//			for(uint16_t i=0;i<65530;i++)
//			{
//				SX126xGetMultipleRandom32Bit(&random,1);
//				//SX126xGetMultipleRandom32Bit()
//				randNum=random%2;
//				HAL_UART_Transmit(&huart1,&randNum,1,1000);
//				//osDelay(10);
//			}


		}
		else if (ReceiveData.Data == DATA_STATE_DEVICE_INIT_OFF)
		{
			RU_CommandProcess(DATA_CMD_TURN_OFF,GlobalData,&ReceiveData);
			osThreadYield();

			LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_2);//interrupt from Semtech - only one IRQ
			LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_2);
			LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_2);	//interrupt from Semtech - only one IRQ

			SendData.Address = ADDR_CORE_RF_STATE_DONE;
			SendData.Data = DATA_STATE_OFF_DONE;
			SendData.pointer = NULL;
			xQueueSend(QueueCoreHandle, &SendData, portMAX_DELAY);

			StateAutomat->PreviousState = StateAutomat->ActualState;
			StateAutomat->ActualState = STATE_OFF;

		}
		break;

	default:
		break;

	}

    return 0;
}

/**
 *
 * @param ReceiveData
 * @param GlobalData
 * @param StateAutomat
 * @param PointerMalloc
 * @return
 */
static uint8_t RF_StateStartON(DATA_QUEUE ReceiveData,
		tDataState_Task_RF* GlobalData, STRUCT_STATE_AUTOMAT_RF* StateAutomat,
		void** PointerMalloc)
{
	return 0;
}

/**
 *
 * @param ReceiveData
 * @param GlobalData
 * @param StateAutomat
 * @param PointerMalloc
 * @return
 */
static uint8_t RF_StateON(DATA_QUEUE ReceiveData,tDataState_Task_RF* GlobalData, STRUCT_STATE_AUTOMAT_RF* StateAutomat,	void** PointerMalloc)
{
	DATA_QUEUE SendData;
    SendData.pointer = NULL;

    typedef struct
	{
	    bool vymerujeme;
	    void* PointerMalloc;
	} STATIC_STRUCTURE, *P_STATIC_STRUCTURE;

    P_STATIC_STRUCTURE LocalStaticStructure;

    if (*PointerMalloc == NULL)
	{
		LocalStaticStructure = (P_STATIC_STRUCTURE) pvPortMalloc(
			sizeof(STATIC_STRUCTURE));
		LocalStaticStructure->PointerMalloc = false;
		LocalStaticStructure->vymerujeme = false;

		*PointerMalloc = (void*) LocalStaticStructure;
	}
    else
	LocalStaticStructure = (P_STATIC_STRUCTURE) *PointerMalloc;

    /* Get RF state*/
    GlobalData->RF_State = RadioGetStatus();

	switch (ReceiveData.Address)
	{
		case ADDR_TO_RF_TASK_CMD:			//Core CMD ADDR
			RU_CommandProcess(ReceiveData.Data,GlobalData,&ReceiveData);
			break;

		case ADDR_SX1276_IRQ:		//Semtech IRQ ADDR
			RU_IRQProcess(GlobalData);

			break;

		case ADDR_CORE_STATE_DEVICE:
			if (ReceiveData.Data == DATA_STATE_DEVICE_INIT_OFF)
			{
				RU_CommandProcess(DATA_CMD_TURN_OFF,GlobalData,&ReceiveData);

				osThreadYield();

				SendData.Address = ADDR_CORE_RF_STATE_DONE;
				SendData.Data = DATA_STATE_OFF_DONE;
				SendData.pointer = NULL;
				xQueueSend(QueueCoreHandle, &SendData, portMAX_DELAY);

				StateAutomat->PreviousState = StateAutomat->ActualState;
				StateAutomat->ActualState = STATE_OFF;

				vPortFree(LocalStaticStructure);
				*PointerMalloc=NULL;

			}
			break;

		default:
			break;
	}


    return 0;
}

/**
 *
 * @param ReceiveData
 * @param GlobalData
 * @param StateAutomat
 * @param PointerMalloc
 * @return
 */
static uint8_t RF_StateStartOFF(DATA_QUEUE ReceiveData,
		tDataState_Task_RF* GlobalData, STRUCT_STATE_AUTOMAT_RF* StateAutomat,
		void** PointerMalloc)
{
	return 0;
}


/**
 *
 */
static void WatchdogRefreshOK_RF (void)
{
  DATA_QUEUE SendData;
  SendData.pointer = NULL;

  SendData.Address = ADDR_CORE_REFRESH_WATCHDOG_OK;
  SendData.Data = DATA_WD_REFRESH_OK_RF;
  xQueueSendToFront(QueueCoreHandle, &SendData, portMAX_DELAY);
}

/**
 *
 * @param argument
 */
void RFTask (void const * argument)
{
  static DATA_QUEUE ReceiveData;
  static tDataState_Task_RF DataTaskRF;
  static STRUCT_STATE_AUTOMAT_RF StateAutomat =    { STATE_OFF, STATE_OFF };
  static void* PointerToMalloc = NULL;
  static portBASE_TYPE ReturnValue;

  /* SX1276 Init*/

  RU_SX1262Assign();
//  RFInit();
#if (TRC_USE_TRACEALYZER_RECORDER == 1)
  myChannel = xTraceRegisterString("RF_TASK");
#endif

  for(uint8_t led=0;led<6;led++)
  {
	  LL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
	  osDelay(100);
  }

 //SET Continous TX
//  RadioSetTxContinuousWave(FREQ_869_525_MHZ,22,0);
//for(;;);
// RFSetTXDown();
//  SX126xSetTxInfinitePreamble();
//  for(;;);


  for (;;)
  {
    ReturnValue = xQueueReceive(QueueRFHandle, &ReceiveData, portMAX_DELAY);
    if (ReturnValue == pdPASS)
	{

#ifdef RUN_WITH_WATCHDOG
	  if (ReceiveData.Address==ADDR_RF_REFRESH_WATCHDOG)
	    {
#if (TRC_USE_TRACEALYZER_RECORDER == 1)
	      vTracePrint(myChannel,"Refreshujeme WD v RF_Task");
#endif
	      WatchdogRefreshOK_RF();
	    }
	  else
#endif // endif RUN WATCHDOG
	    {
	      StateRF[StateAutomat.ActualState] (ReceiveData, &DataTaskRF,&StateAutomat,&PointerToMalloc);
	    }

	  /* Clear malloc */
	  vPortFree (ReceiveData.pointer);
	  ReceiveData.pointer = NULL;
	}

  }
}
