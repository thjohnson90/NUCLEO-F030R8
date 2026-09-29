/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "2004a_i2c_lcd.h"
#include "dht11.h"

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

/* USER CODE BEGIN PV */
static char        uart_msg[MAX_UART_MSG_LEN] = {0};
static char        uart_cmd[MAX_UART_MSG_LEN - 15] = {0};
static uint32_t    uart_msg_len               = 0;
static uint32_t    uart_cmd_len               = 0;
static float       rh                         = 0.0;
static float       temp_C                     = 0.0;
static float       temp_F                     = 0.0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void UpdateLCDDisplay(DHT11_Device* dht, uint16_t lcd_device_addr);
static void UpdateUARTMessages(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#ifdef LOG_IN_MAIN_LOOP
extern uint32_t detected_pw;
extern uint32_t rise_ts;
extern uint32_t fall_ts;
#endif

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	HAL_StatusTypeDef ret                        = HAL_ERROR;
	uint16_t          lcd_device_addr            = 0;
	uint16_t          idx                        = 0;
	DHT11_Device      dht11                      = {0};
	uint32_t          start_tick                 = 0;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_TIM6_Init();
  MX_TIM14_Init();
  MX_TIM15_Init();
  /* USER CODE BEGIN 2 */

  // start timer
  HAL_TIM_Base_Start(&htim6);
  HAL_TIM_IC_Start_IT(&htim15, TIM_CHANNEL_1);
#ifdef PW_MEASURE_TEST
  HAL_TIM_PWM_Start(&htim14, TIM_CHANNEL_1);
#endif
  HAL_Delay(5);
  DHT11_Init(&dht11, &htim6);

  // introductory message
  sprintf(uart_msg, "USART2 Initialized.\r\n");
  uart_msg_len = strlen(uart_msg);
  ret = HAL_UART_Transmit(&huart2, (const uint8_t*) uart_msg, uart_msg_len, FUNC_TIMEOUT);
  if (HAL_OK != ret) {
	  sprintf(uart_msg, "USART2 Error.\r\n");
	  uart_msg_len = strlen(uart_msg);
	  HAL_UART_Transmit(&huart2, (const uint8_t*) uart_msg, uart_msg_len, FUNC_TIMEOUT);
  }

  // search for LCD display
  for (idx = 0; idx < MAX_I2C_DEVICES; idx++) {
	  ret = HAL_I2C_IsDeviceReady(&hi2c1, (idx << 1), MAX_I2C_RETRY, FUNC_TIMEOUT);
	  if (HAL_OK == ret) {
		  sprintf(uart_msg, "Found I2C Device at Address 0x%x\r\n", idx);
		  uart_msg_len = strlen(uart_msg);
		  HAL_UART_Transmit(&huart2, (const uint8_t*) uart_msg, uart_msg_len, FUNC_TIMEOUT);

		  if (idx == LCD_DEVICE_ADDR) {
			  sprintf(uart_msg, "LCD Found at Address 0x%x\r\n", idx);
			  uart_msg_len = strlen(uart_msg);
			  HAL_UART_Transmit(&huart2, (const uint8_t*) uart_msg, uart_msg_len, FUNC_TIMEOUT);
			  lcd_device_addr = (idx << 1);
			  break;
		  }
	  }
  }

  if (idx != MAX_I2C_DEVICES) {
	  // Initialize LCD Display
	  LCD_Init(&hi2c1, lcd_device_addr);
  }

//  delay_us(100);

  memset(uart_cmd, 0, sizeof(uart_cmd));
  uart_cmd_len = 0;
  HAL_UART_Receive_IT(&huart2, (uint8_t*) uart_msg, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_Delay(5000);
  ReadDHT11(&dht11);
  start_tick = HAL_GetTick();
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	// create a non-blocking delay by reading the system clock
	if (HAL_GetTick() - start_tick > 5000) {		// 5s elapsed
		start_tick = HAL_GetTick();
		UpdateLCDDisplay(&dht11, lcd_device_addr);
		UpdateUARTMessages();
	}
  }

  HAL_TIM_Base_Stop(&htim6);
  HAL_TIM_IC_Stop_IT(&htim15, TIM_CHANNEL_1);
#ifdef PW_MEASURE_TEST
  HAL_TIM_PWM_Stop(&htim14, TIM_CHANNEL_1);
#endif

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV16;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void UpdateLCDDisplay(DHT11_Device* dht, uint16_t lcd_device_addr)
{
	char     lcd_msg[MAX_LCD_MSG_LEN]   = {0};
	uint32_t lcd_msg_len                = 0;

	ReadDHT11(dht);
	LCD_ClearDisplay(&hi2c1, lcd_device_addr);

	rh = ((float) dht->rh_int) + ((float) dht->rh_dec) / 1000.0f;
	temp_C = ((float) dht->temp_int) + ((float) dht->temp_dec) / 1000.0f;
	temp_F = (temp_C * 1.8f) + 32.0f;

	HAL_Delay(1);  // required for LCD display to work

	sprintf(lcd_msg, "Rel Humidity: %02.1f%%", rh);
	lcd_msg_len = strlen(lcd_msg);
	LCD_WriteData(&hi2c1, lcd_device_addr, (uint8_t*) lcd_msg, lcd_msg_len);

	LCD_SetDDRAMAddr(&hi2c1, lcd_device_addr, LCD_DISP_LINE1_START);
	sprintf(lcd_msg, "Temperature:");
	lcd_msg_len = strlen(lcd_msg);
	LCD_WriteData(&hi2c1, lcd_device_addr, (uint8_t*) lcd_msg, lcd_msg_len);

	LCD_SetDDRAMAddr(&hi2c1, lcd_device_addr, LCD_DISP_LINE2_START);
	sprintf(lcd_msg, "%02.1f %cC", temp_C, 0xDF);
	lcd_msg_len = strlen(lcd_msg);
	LCD_WriteData(&hi2c1, lcd_device_addr, (uint8_t*) lcd_msg, lcd_msg_len);

	LCD_SetDDRAMAddr(&hi2c1, lcd_device_addr, LCD_DISP_LINE3_START);
	sprintf(lcd_msg, "%02.1f %cF", temp_F, 0xDF);
	lcd_msg_len = strlen(lcd_msg);
	LCD_WriteData(&hi2c1, lcd_device_addr, (uint8_t*) lcd_msg, lcd_msg_len);
}

void UpdateUARTMessages(void)
{
#if 0
	char              uart_msg[MAX_UART_MSG_LEN] = {0};
	uint32_t          uart_msg_len               = 0;
	double            period                     = 0.0L;

	sprintf(uart_msg, "Rel Hum: %2.1f%%\r\n", rh);
	uart_msg_len = strlen(uart_msg);
	HAL_UART_Transmit(&huart2, (const uint8_t*) uart_msg, uart_msg_len, HAL_TIMEOUT);

	sprintf(uart_msg, "Temp: %02.2f C, %02.2f F\r\n\r\n", temp_C, temp_F);
	uart_msg_len = strlen(uart_msg);
	HAL_UART_Transmit(&huart2, (const uint8_t*) uart_msg, uart_msg_len, HAL_TIMEOUT);

	period = 1.0f / TIMER_INPUT_CLOCK_FREQ;
	sprintf(uart_msg, "Timer Clock Period: %02.16f\r\n", period);
	uart_msg_len = strlen(uart_msg);
	HAL_UART_Transmit(&huart2, (const uint8_t*) uart_msg, uart_msg_len, HAL_TIMEOUT);
#endif

#ifdef PW_MEASURE_TEST
#ifdef LOG_IN_MAIN_LOOP
	sprintf(uart_msg, "Pulse Width %05u:%05u:%05u\r\n", (unsigned) rise_ts, (unsigned) fall_ts, (unsigned) detected_pw);
	uart_msg_len = strlen(uart_msg);
	HAL_UART_Transmit(&huart2, (const uint8_t*) uart_msg, uart_msg_len, HAL_TIMEOUT);
#endif
#endif
}

// UART2 Receive Callback
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
	if (&huart2 == huart) {
		if (uart_msg[0] == '\r') {
			sprintf(uart_msg, "\r\n");
			uart_msg_len = strlen(uart_msg);
			HAL_UART_Transmit(huart, (const uint8_t*) uart_msg, uart_msg_len, HAL_TIMEOUT);

			sprintf(uart_msg, "Command: %s\r\n", uart_cmd);
			uart_msg_len = strlen(uart_msg);
			HAL_UART_Transmit(huart, (const uint8_t*) uart_msg, uart_msg_len, HAL_TIMEOUT);
			memset(uart_cmd, 0, sizeof(uart_cmd));
			uart_cmd_len = 0;
		} else {
			HAL_UART_Transmit(huart, (const uint8_t*) uart_msg, 1, HAL_TIMEOUT);
			uart_cmd[uart_cmd_len] = uart_msg[0];
			uart_cmd_len++;
		}
	}
	HAL_UART_Receive_IT(huart, (uint8_t*) uart_msg, 1);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
