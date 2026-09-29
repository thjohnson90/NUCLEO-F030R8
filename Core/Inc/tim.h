/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern TIM_HandleTypeDef htim6;

extern TIM_HandleTypeDef htim14;

extern TIM_HandleTypeDef htim15;

/* USER CODE BEGIN Private defines */
#define PW_MEASURE_TEST
#ifdef PW_MEASURE_TEST
//#define LOG_IN_IRQ_CALLBACK
//#define LOG_IN_MAIN_LOOP
#endif

#define SYS_CLK_FREQ            48000000.0f
#define TIMER_INPUT_CLOCK_FREQ	6000000.0f
#define TIM6_PRESCALER			3.0f
#define TIM6_PERIOD				(1.0f / (TIMER_INPUT_CLOCK_FREQ / TIM6_PRESCALER))
#define TIM14_PRESCALER			600.0f
#define TIM14_PERIOD			(1.0f / (TIMER_INPUT_CLOCK_FREQ / TIM14_PRESCALER))

/* USER CODE END Private defines */

void MX_TIM6_Init(void);
void MX_TIM14_Init(void);
void MX_TIM15_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* USER CODE BEGIN Prototypes */
void delay_us(uint16_t us);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */

