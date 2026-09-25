/*
 * dht11.h
 *
 *  Created on: Sep 24, 2026
 *      Author: tmjohns1
 */

#ifndef INC_DHT11_H_
#define INC_DHT11_H_

typedef struct _dht11_device {
	GPIO_TypeDef*      port;
	GPIO_InitTypeDef   gpio_input;
	GPIO_InitTypeDef   gpio_output;
	TIM_HandleTypeDef* timer;
	uint8_t            temp_int;
	uint8_t            temp_dec;
	uint8_t            rh_int;
	uint8_t            rh_dec;
} DHT11_Device;

void     DHT11_Init(DHT11_Device* dht11, TIM_HandleTypeDef* htim);
uint32_t ReadDHT11(DHT11_Device* dht11);

#endif /* INC_DHT11_H_ */
