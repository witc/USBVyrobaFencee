/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include <stdbool.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include "stm32l0xx_ll_exti.h"
#include "stm32l0xx_ll_gpio.h"
#include "stm32l0xx_ll_dma.h"
#include "stm32l0xx_ll_usart.h"
#include "stm32l0xx_ll_tim.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
void LogError(uint64_t code);
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define AUX_8_Pin GPIO_PIN_2
#define AUX_8_GPIO_Port GPIOA
#define AUX_7_Pin GPIO_PIN_3
#define AUX_7_GPIO_Port GPIOA
#define SX1262_NSS_Pin GPIO_PIN_4
#define SX1262_NSS_GPIO_Port GPIOA
#define SX1262_SCK_Pin GPIO_PIN_5
#define SX1262_SCK_GPIO_Port GPIOA
#define SX1262_MISO_Pin GPIO_PIN_6
#define SX1262_MISO_GPIO_Port GPIOA
#define SX1262_MOSI_Pin GPIO_PIN_7
#define SX1262_MOSI_GPIO_Port GPIOA
#define SX1262_RESET_Pin GPIO_PIN_1
#define SX1262_RESET_GPIO_Port GPIOB
#define SX1262_DIO1_Pin GPIO_PIN_2
#define SX1262_DIO1_GPIO_Port GPIOB
#define SX1262_DIO1_EXTI_IRQn EXTI2_3_IRQn
#define SX1262_BUSY_Pin GPIO_PIN_10
#define SX1262_BUSY_GPIO_Port GPIOB
#define RF_SWITCH_Pin GPIO_PIN_11
#define RF_SWITCH_GPIO_Port GPIOB
#define AUX_8B13_Pin GPIO_PIN_13
#define AUX_8B13_GPIO_Port GPIOB
#define AUX_7B14_Pin GPIO_PIN_14
#define AUX_7B14_GPIO_Port GPIOB
#define LED_GREEN_Pin GPIO_PIN_15
#define LED_GREEN_GPIO_Port GPIOB
#define SX1262_RF_SWITCH_Pin GPIO_PIN_8
#define SX1262_RF_SWITCH_GPIO_Port GPIOA
#define AUX_4_Pin GPIO_PIN_12
#define AUX_4_GPIO_Port GPIOA
#define AUX_3_Pin GPIO_PIN_3
#define AUX_3_GPIO_Port GPIOB
#define AUX_2_Pin GPIO_PIN_4
#define AUX_2_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_5
#define LED1_GPIO_Port GPIOB
#define AUX_6_Pin GPIO_PIN_6
#define AUX_6_GPIO_Port GPIOB
#define AUX_6_EXTI_IRQn EXTI4_15_IRQn
#define AUX_5_Pin GPIO_PIN_7
#define AUX_5_GPIO_Port GPIOB
#define AUX_5_EXTI_IRQn EXTI4_15_IRQn
#define AUX_1_Pin GPIO_PIN_9
#define AUX_1_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */



//Data  CMD for RF task
#define DATA_CMD_WAIT_FOR_PAIR				0
#define DATA_CMD_PAIR_OK					1
#define DATA_CMD_SEND_REQUEST_INFO			3
#define DATA_CMD_END_OF_PAIRING				4
#define DATA_CMD_SEND_NEGATE_STATE			5
#define DATA_CMD_SEND_PAIRING_REQUEST		6
#define DATA_CMD_SEND_NEGATE_POWER_LEVEL	7
#define	DATA_CMD_START_CAD					8
#define DATA_CMD_SEND_ALARM_LVL				10
#define DATA_CMD_DISSABLE_CAD				11
#define DATA_CMD_ENABLE_CAD					12
#define DATA_CMD_START_RX_TIMEOUT			13
#define DATA_CMD_VYMEROVACI_PAKET			14
#define DATA_CMD_BETTER_RX_ASYNC_PO_ZAP		15
#define DATA_CMD_ANNOUNCEMENT				16
#define DATA_CMD_PAUSE_RX					17
#define DATA_CMD_START_RX					18
#define DATA_CMD_TURN_OFF					19
#define DATA_CMD_RF_INIT_ON					20



/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
