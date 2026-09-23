/*
 * hd44780u.c
 *
 *  Created on: Sep 15, 2026
 *      Author: tmjohns1
 */

#include <2004a_i2c_lcd.h>
#include "stm32f0xx_hal.h"
#include "tim.h"

HAL_StatusTypeDef WriteInstNibble(I2C_HandleTypeDef i2cdev, uint32_t dev_addr, uint8_t* pdata)
{
	uint8_t           tmp    = 0;
	HAL_StatusTypeDef status = HAL_ERROR;

	// write command nibble
	tmp = (*pdata) | (BIT_INST | BIT_WRITE | BIT_NOENABLE | BACKLIGHT_OFF);
	status = HAL_I2C_Master_Transmit(&i2cdev, dev_addr, &tmp, INST_NIBBLE_SZ, FUNC_TIMEOUT);
	tmp = (*pdata) | (BIT_INST | BIT_WRITE | BIT_ENABLE | BACKLIGHT_OFF);
	status = HAL_I2C_Master_Transmit(&i2cdev, dev_addr, &tmp, INST_NIBBLE_SZ, FUNC_TIMEOUT);
	tmp = (*pdata) | (BIT_INST | BIT_WRITE | BIT_NOENABLE | BACKLIGHT_OFF);
	status = HAL_I2C_Master_Transmit(&i2cdev, dev_addr, &tmp, INST_NIBBLE_SZ, FUNC_TIMEOUT);
	delay_us(INST_EXEC_DELAY);

	return status;
}

HAL_StatusTypeDef WriteInst(I2C_HandleTypeDef i2cdev, uint32_t dev_addr, uint8_t* pdata)
{
	uint8_t           hi     = 0;
	uint8_t           lo     = 0;
	HAL_StatusTypeDef status = HAL_ERROR;

	// get high and low nibbles
	hi = GetHiNibble(pdata);
	lo = GetLoNibble(pdata);

	// write upper command nibble
	status = WriteInstNibble(i2cdev, dev_addr, &hi);

	// write lower command nibble
	status = WriteInstNibble(i2cdev, dev_addr, &lo);

	return status;
}

HAL_StatusTypeDef WriteDataNibble(I2C_HandleTypeDef i2cdev, uint32_t dev_addr, uint8_t* pdata)
{
	uint8_t           tmp    = 0;
	HAL_StatusTypeDef status = HAL_ERROR;

	// write data nibble
	tmp = *pdata | (BIT_DATA | BIT_WRITE | BIT_NOENABLE | BACKLIGHT_OFF);
	status = HAL_I2C_Master_Transmit(&i2cdev, dev_addr, &tmp, DATA_NIBBLE_SZ, FUNC_TIMEOUT);
	tmp = *pdata | (BIT_DATA | BIT_WRITE | BIT_ENABLE | BACKLIGHT_OFF);
	status = HAL_I2C_Master_Transmit(&i2cdev, dev_addr, &tmp, DATA_NIBBLE_SZ, FUNC_TIMEOUT);
	tmp = *pdata | (BIT_DATA | BIT_WRITE | BIT_NOENABLE | BACKLIGHT_ON);
	status = HAL_I2C_Master_Transmit(&i2cdev, dev_addr, &tmp, DATA_NIBBLE_SZ, FUNC_TIMEOUT);
	delay_us(INST_EXEC_DELAY);

	return status;
}

HAL_StatusTypeDef WriteData(I2C_HandleTypeDef i2cdev, uint32_t dev_addr, uint8_t* pdata, uint32_t sz)
{
	uint32_t          idx    = 0;
	uint8_t           hi     = 0;
	uint8_t           lo     = 0;
	HAL_StatusTypeDef status = HAL_ERROR;

	// iterate through buffer
	for (idx = 0; idx < sz; idx++) {
		// get high and low nibbles
		hi = GetHiNibble(&pdata[idx]);
		lo = GetLoNibble(&pdata[idx]);

		// write upper data nibble
		status = WriteDataNibble(i2cdev, dev_addr, &hi);

		// write lower data nibble
		status = WriteDataNibble(i2cdev, dev_addr, &lo);
	}

	return status;
}

void LCD_Init(uint16_t addr)
{
	uint8_t           inst = 0;
	uint8_t           tmp  = 0;

	// 4-bit mode, multi-line, 5x8 font
	inst = FUNC_SET | BIT_4BIT | BIT_4LINES | BIT_5X8FONT;
	tmp = inst & UPPER_NIBBLE_MASK;
	WriteInstNibble(hi2c1, addr, &tmp);
	WriteInst(hi2c1, addr, &inst);

	// turn on display
	inst = DISP_ON_OFF | BIT_DISPLAY_ON | BIT_CURSOR_ON | BIT_UNDERLINE_CURS;
	WriteInst(hi2c1, addr, &inst);

	// clear display
	inst = CLR_DISP;
	WriteInst(hi2c1, addr, &inst);

	// set entry mode
	inst = ENTRY_MODE | BIT_INC_CURS_POS | BIT_PAIR_DISP_SHIFT_OFF;
	WriteInst(hi2c1, addr, &inst);
}
