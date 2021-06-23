


#ifndef __GLDEF_H
#define __GLDEF_H


typedef struct
{
  uint8_t Address;
  uint32_t Data;
  uint32_t RFU;
  uint8_t temp;
  void* pointer;

} DATA_QUEUE;

#define ADDR_CORE_STATE_DEVICE				0

// ADDRESS for Core
#define ADDR_CORE_DISPLAY_STATE_DONE			255
#define ADDR_CORE_RF_STATE_DONE					254
#define ADDR_CORE_RF_DATA_RECEIVED				253
#define ADDR_CORE_SWITCH_SHORT_EVENT		 	252
#define ADDR_CORE_SWITCH_LONG_EVENT				251
#define ADDR_CORE_SWITCH_EXTRA_LONG_EVENT	 	250
#define ADDR_CORE_REFRESH_WATCHDOG_OK		 	249
#define ADDR_CORE_TESTING_RSSI					248
#define ADDR_CORE_TX_TEST_DONE					247
#define ADDR_CORE_IAM_IN_MEASURING_MODE			246
#define ADDR_CORE_SWITCH_REPEAT_EVENT	 		245
#define ADDR_CORE_RF_ANSWER_TIMEOUT				244
#define ADDR_CORE_SET_OUT_OF_RANGE				243
#define ADDR_CORE_ELAPSEDTIME_1_SEC				242
#define ADDR_CORE_ALARM_AUTO_OFF				241
#define ADDR_CORE_TURN_OFF_EVENT				240
#define ADDR_CORE_USART_RX_DONE					239
#define ADDR_CORE_BUTTON_PUSHED					238
#define ADDR_TO_CORE_READ_AUX6_PIN				237
#define ADDR_TO_CORE_UART_READ_RX_BUFFER		236

/****************************************************
 *  ADDRESSES only Sent (used) for TaskRF
 ***************************************************/
#define ADDR_SX1276_IRQ						255
#define ADDR_TIMER_RF_IRQ					254
#define ADDR_TO_RF_TASK_CMD					253
#define ADDR_RF_START_MEAS_BATT				252
#define	ADDR_RF_REFRESH_WATCHDOG			251


/* Data State from ADDR_core */
#define DATA_STATE_DEVICE_OFF				1
#define DATA_STATE_DEVICE_ON				2
#define DATA_STATE_DEVICE_INIT_OFF			3
#define DATA_STATE_DEVICE_INIT_ON			4
#define DATA_STATE_DEVICE_ONLY_RF_ON		5


#define DATA_WD_REFRESH_OK_RF				1
#define DATA_WD_REFRESH_OK_LCD				2
#define DATA_STATE_ON_DONE					3
#define DATA_STATE_OFF_DONE					4
#define DATA_STATE_ONLY_RF_DONE				5

#endif
