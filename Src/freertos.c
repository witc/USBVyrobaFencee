/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gldef.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId CoreTaskHandle;
uint32_t CoreTaskBuffer[ 128 ];
osStaticThreadDef_t CoreTaskControlBlock;
osThreadId RFTaskHandle;
uint32_t RFTaskBuffer[ 200 ];
osStaticThreadDef_t RFTaskControlBlock;
osMessageQId QueueCoreHandle;
uint8_t QueueCoreBuffer[ 16 * sizeof( DATA_QUEUE ) ];
osStaticMessageQDef_t QueueCoreControlBlock;
osMessageQId QueueRFHandle;
uint8_t QueueRFBuffer[ 16 * sizeof( DATA_QUEUE ) ];
osStaticMessageQDef_t QueueRFControlBlock;
osTimerId TimerLEDHandle;
osStaticTimerDef_t TimerLEDControlBlock;
osTimerId TimerButtonHandle;
osStaticTimerDef_t TimerButtonControlBlock;
osTimerId TimerSystemLEDHandle;
osStaticTimerDef_t TimerSystemLEDControlBlock;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
   
/* USER CODE END FunctionPrototypes */

void StartTaskCore(void const * argument);
void StartRfTask(void const * argument);
void Callback_LED(void const * argument);
void CallbackButtonCheck(void const * argument);
void CallbackSystemLED(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];
  
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}                   
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];
  
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )  
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}                   
/* USER CODE END GET_TIMER_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* definition and creation of TimerLED */
  osTimerStaticDef(TimerLED, Callback_LED, &TimerLEDControlBlock);
  TimerLEDHandle = osTimerCreate(osTimer(TimerLED), osTimerPeriodic, NULL);

  /* definition and creation of TimerButton */
  osTimerStaticDef(TimerButton, CallbackButtonCheck, &TimerButtonControlBlock);
  TimerButtonHandle = osTimerCreate(osTimer(TimerButton), osTimerOnce, NULL);

  /* definition and creation of TimerSystemLED */
  osTimerStaticDef(TimerSystemLED, CallbackSystemLED, &TimerSystemLEDControlBlock);
  TimerSystemLEDHandle = osTimerCreate(osTimer(TimerSystemLED), osTimerOnce, NULL);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of QueueCore */
  osMessageQStaticDef(QueueCore, 16, DATA_QUEUE, QueueCoreBuffer, &QueueCoreControlBlock);
  QueueCoreHandle = osMessageCreate(osMessageQ(QueueCore), NULL);

  /* definition and creation of QueueRF */
  osMessageQStaticDef(QueueRF, 16, DATA_QUEUE, QueueRFBuffer, &QueueRFControlBlock);
  QueueRFHandle = osMessageCreate(osMessageQ(QueueRF), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of CoreTask */
  osThreadStaticDef(CoreTask, StartTaskCore, osPriorityNormal, 0, 128, CoreTaskBuffer, &CoreTaskControlBlock);
  CoreTaskHandle = osThreadCreate(osThread(CoreTask), NULL);

  /* definition and creation of RFTask */
  osThreadStaticDef(RFTask, StartRfTask, osPriorityNormal, 0, 200, RFTaskBuffer, &RFTaskControlBlock);
  RFTaskHandle = osThreadCreate(osThread(RFTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartTaskCore */
/**
  * @brief  Function implementing the CoreTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartTaskCore */
void StartTaskCore(void const * argument)
{
  /* USER CODE BEGIN StartTaskCore */
	TaskCore(argument);
  /* Infinite loop */

  /* USER CODE END StartTaskCore */
}

/* USER CODE BEGIN Header_StartRfTask */
/**
* @brief Function implementing the RFTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartRfTask */
void StartRfTask(void const * argument)
{
#if (USE_WITH_RF == true)
  /* USER CODE BEGIN StartRfTask */
  /* Infinite loop */
  RFTask(argument);
  /* USER CODE END StartRfTask */
#else
  osDelay(UINT32_MAX);
#endif
}

/* Callback_LED function */
__weak void Callback_LED(void const * argument)
{
  /* USER CODE BEGIN Callback_LED */

  /* USER CODE END Callback_LED */
}

/* CallbackButtonCheck function */
__weak void CallbackButtonCheck(void const * argument)
{
  /* USER CODE BEGIN CallbackButtonCheck */

  /* USER CODE END CallbackButtonCheck */
}

/* CallbackSystemLED function */
__weak void CallbackSystemLED(void const * argument)
{
  /* USER CODE BEGIN CallbackSystemLED */

  /* USER CODE END CallbackSystemLED */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
