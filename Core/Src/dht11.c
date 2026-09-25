/*
 * dht11.c
 *
 *  Created on: Sep 24, 2026
 *      Author: tmjohns1
 */

#include <stm32f0xx_hal.h>
#include <stm32f0xx_hal_gpio.h>
#include <string.h>
#include <stdio.h>

#include "main.h"
#include "dht11.h"
#include "usart.h"
#include "tim.h"

//#define DEBUG_DHT11

static uint32_t start_time  = 0;
static uint32_t pulse_width = {0};
#ifdef DEBUG_DHT11
static double pwf[40] = {0.0f};
#endif

static inline void SetDHT11GPIOAsInput(DHT11_Device* dht11)
{
	HAL_GPIO_Init(dht11->port, &dht11->gpio_input);
}

static inline void SetDHT11GPIOAsOutput(DHT11_Device* dht11)
{
	HAL_GPIO_Init(dht11->port, &dht11->gpio_output);
}

static inline void DetectNegativeEdge(DHT11_Device* dht11, uint32_t max_delay)
{
	// TIM6 ticks at 16MHz

	__HAL_TIM_SET_COUNTER(dht11->timer, 0);
	while (HAL_GPIO_ReadPin(dht11->port, dht11->gpio_input.Pin) == GPIO_PIN_SET) {
		if (__HAL_TIM_GET_COUNTER(dht11->timer) > max_delay) {
			return;
		}
	}
}

static inline void DetectPositiveEdge(DHT11_Device* dht11, uint32_t max_delay)
{
	// TIM6 ticks at 1MHz

	__HAL_TIM_SET_COUNTER(dht11->timer, 0);
	while (HAL_GPIO_ReadPin(dht11->port, dht11->gpio_input.Pin) == GPIO_PIN_RESET) {
		if (__HAL_TIM_GET_COUNTER(dht11->timer) > max_delay) {
			return;
		}
	}
}

static inline void DetectPulseWidth(DHT11_Device* dht11, uint8_t* data, uint32_t idx)
{
	// get beginning timestamp
	start_time = __HAL_TIM_GET_COUNTER(dht11->timer);

	// detect the negative edge
	while (HAL_GPIO_ReadPin(dht11->port, dht11->gpio_input.Pin) == GPIO_PIN_SET) {
		if ((pulse_width = __HAL_TIM_GET_COUNTER(dht11->timer)) > 2000) {		// data pulse width should be no more than 70us (limit set to 126us)
			return;
		}
	}

	pulse_width -= start_time;

#ifdef DEBUG_DHT11
	pwf[idx] = (double) pulse_width * (double) 0.063;		// TIM6 ticks at 16MHz
#endif

	// record data received
	if (pulse_width > 238 && pulse_width < 635) {		// '0' indicated by pulse width between 26-28us (limits set at 15us and 40us)
		// received '0'
		data[idx] = 0;
	} else if (pulse_width > 873 && pulse_width < 1270) {	// '1' indicated by pulse width of 70us (limits set at 55us and 80us)
		// received '1'
		data[idx] = 1;
	} else {
		// bad data
		return;
	}
}

void DHT11_Init(DHT11_Device* dht11, TIM_HandleTypeDef* htim)
{
	// clear data structure
	memset(dht11, 0, sizeof(DHT11_Device));

	// initialize GPIO port setting
	dht11->port = DHT11_Data_GPIO_Port;

	// initialization for GPIO as output
	dht11->gpio_output.Pin      = DHT11_Data_Pin;
	dht11->gpio_output.Mode     = GPIO_MODE_OUTPUT_PP;
	dht11->gpio_output.Pull     = GPIO_NOPULL;
	dht11->gpio_output.Speed    = GPIO_SPEED_FREQ_HIGH;

	// initialization for GPIO as input
	dht11->gpio_input.Pin       = DHT11_Data_Pin;
	dht11->gpio_input.Mode      = GPIO_MODE_INPUT;
	dht11->gpio_input.Pull      = GPIO_NOPULL;
	dht11->gpio_input.Speed     = GPIO_SPEED_FREQ_HIGH;

	// set associated timer
	dht11->timer = htim;
}

uint32_t ReadDHT11(DHT11_Device* dht11)
{
	uint8_t  data[40]  = {0};			// DHT11 transmits 40 bytes of data
	uint32_t idx       = 0;
	uint32_t xmit_xsum = 0;
	uint32_t calc_xsum = 0;
#ifdef DEBUG_DHT11
	char uart_msg[128] = {0};
	uint32_t uart_msg_len = 0;
	double tmp1, tmp2, tmp3, tmp4;
#endif

	memset(data, -1, sizeof(data));

	// DHT11 GPIO to input mode and verify data bus is available.
	SetDHT11GPIOAsInput(dht11);
	DetectPositiveEdge(dht11, 10000);

	// set DHT11 GPIO to output mode
	SetDHT11GPIOAsOutput(dht11);

	// request DHT11 data with 18ms low pulse
	HAL_GPIO_WritePin(dht11->port, dht11->gpio_output.Pin, GPIO_PIN_RESET);
	HAL_Delay(20);
	HAL_GPIO_WritePin(dht11->port, dht11->gpio_output.Pin, GPIO_PIN_SET);

	// disable interrupts
//	__disable_irq();

	// TIM6 ticks at 16MHz

	// clear counter and wait for 20-40us high response from DHT11
	SetDHT11GPIOAsInput(dht11);
	DetectNegativeEdge(dht11, 794);		// set 50us max delay

	// wait for positive edge - low pulse should be ~80us
	DetectPositiveEdge(dht11, 1429);		// set 90us max delay

	// wait for negative edge - high pulse should be ~80us
	DetectNegativeEdge(dht11, 1429);		// set 90us max delay

	// DHT11 ready to start transmitting data
	for (idx = 0; idx < 40; idx++) {	// DHT11 transmits 40 bytes of data
		// wait for 50us low pulse preceding transmission of one bit of data
		DetectPositiveEdge(dht11, 952);	// set 60us max delay

		// get pulse width (to determine data is '1' or '0')
		DetectPulseWidth(dht11, data, idx);
	}

	// enable interrupts
//	__enable_irq();

#ifdef DEBUG_DHT11
	tmp1 = (double) neg1 * (double) 0.063;
	tmp2 = (double) pos1 * (double) 0.063;
	tmp3 = (double) neg2 * (double) 0.063;
	tmp4 = (double) pos2 * (double) 0.063;

	sprintf(uart_msg, "neg1:pos1:neg2:pos2 %05.2lf, %05.2lf, %05.2lf, %05.2lf\r\n", tmp1, tmp2, tmp3, tmp4);
	uart_msg_len = strlen(uart_msg);
	HAL_UART_Transmit(&huart2, (const uint8_t*) uart_msg, uart_msg_len, HAL_TIMEOUT);
	sprintf(uart_msg, "\r\npw %05.2lf\r\n", (double) pulse_width * (double) 0.063);
	uart_msg_len = strlen(uart_msg);
	HAL_UART_Transmit(&huart2, (const uint8_t*) uart_msg, uart_msg_len, HAL_TIMEOUT);
#endif

	// data returned is:
	//   8 bits integral RH data
	//   8 bits decimal RH data
	//   8 bits integral Temp data
	//   8 bits decimal Temp data
	//   8 bits checksum

	dht11->rh_int = 0;
	for (idx = 0; idx < 8; idx++) {
		dht11->rh_int = (dht11->rh_int << 1) | data[idx];
	}

	dht11->rh_dec = 0;
	for (idx = 8; idx < 16; idx++) {
		dht11->rh_dec = (dht11->rh_dec << 1) | data[idx];
	}

	dht11->temp_int = 0;
	for (idx = 16; idx < 24; idx++) {
		dht11->temp_int = (dht11->temp_int << 1) | data[idx];
	}

	dht11->temp_dec = 0;
	for (idx = 24; idx < 32; idx++) {
		dht11->temp_dec = (dht11->temp_dec << 1) | data[idx];
	}

	xmit_xsum = 0;
	for (idx = 32; idx < 40; idx++) {
		xmit_xsum = (xmit_xsum << 1) | data[idx];
	}

	calc_xsum = dht11->rh_int + dht11->rh_dec + dht11->temp_int + dht11->temp_dec;
	calc_xsum &= 0xFF;

#ifdef DEBUG_DHT11
	for (idx = 0; idx < 40; idx++) {
		sprintf(uart_msg, "pwf[%02d] = %05.2lf\r\n", (unsigned) idx, pwf[idx]);
		uart_msg_len = strlen(uart_msg);
		HAL_UART_Transmit(&huart2, (const uint8_t*) uart_msg, uart_msg_len, HAL_TIMEOUT);
	}
#endif

	if (xmit_xsum != calc_xsum) {
		return -1;
	}

	return 0;
}
