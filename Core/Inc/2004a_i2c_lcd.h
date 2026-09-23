/*
 * hd44780u.h
 *
 *  Created on: Sep 15, 2026
 *      Author: tmjohns1
 */

#ifndef INC_2004A_I2C_LCD_H_
#define INC_2004A_I2C_LCD_H_

#include "i2c.h"

// I2C Slave Device Address
#define MAX_I2C_DEVICES		(128)
#define MAX_I2C_RETRY       (100)
#define LCD_DEVICE_ADDR		(0x27)
//#define LCD_DEVICE_ADDR		(0x3F)

// Command Definitions
#define CLR_DISP			0x01
#define HOME				0x02
#define ENTRY_MODE			0x04
#define DISP_ON_OFF			0x08
#define CUROR_OR_DISP_SHIFT	0x10
#define FUNC_SET			0x20
#define CGRAM_ADDR			0x40
#define DDRAM_ADDR			0x80

// Command Flags
// ENTRY_MODE Flags
#define BIT_INC_CURS_POS		(0x1 << 1)
#define BIT_DEC_CURS_POS		(0x0)
#define BIT_PAIR_DISP_SHIFT_ON	(0x1)
#define BIT_PAIR_DISP_SHIFT_OFF	(0x0)

// DISP_ON_OFF Flags
#define BIT_DISPLAY_ON			(0x1 << 2)
#define BIT_DISPLAY_OFF			(0x0)
#define BIT_CURSOR_ON			(0x1 << 1)
#define BIT_CURSOR_OFF			(0x0)
#define BIT_BLINK_CURS			(0x1)
#define BIT_UNDERLINE_CURS		(0x0)

// CURSOR_OR_DISP_SHIFT
#define BIT_DISPLAY_SHIFT		(0x1 << 3)
#define BIT_CURSOR_MOVE			(0x0)
#define BIT_SHIFT_RIGHT			(0x1 << 2)
#define BIT_SHIFT_LEFT			(0x0)

// FUNC_SET
#define BIT_8BIT				(0x1 << 4)
#define BIT_4BIT				(0x0)
#define BIT_4LINES				(0x1 << 3)
#define BIT_1LINE				(0x0)
#define BIT_5X10FONT			(0x1 << 2)
#define BIT_5X8FONT				(0x0)

// CGRAM_ADDR
#define CGRAM_ADDR_MASK		(0x3F)

// DDRAM_ADDR
#define DDRAM_ADDR_MASK		(0x7F)

// Nibble Masks
#define UPPER_NIBBLE_MASK	(0xF0)
#define LOWER_NIBBLE_MASK	(0x0F)

// Command Bits
#define BIT_ENABLE			(0x1 << 2)
#define BIT_NOENABLE		(0x0)
#define BIT_READ			(0x1 << 1)
#define BIT_WRITE			(0x0)
#define BIT_DATA			(0x1)
#define BIT_INST			(0x0)

#define INST_NIBBLE_SZ		(0x1)
#define DATA_NIBBLE_SZ		(0x1)

#define BACKLIGHT_ON		(0x08)
#define BACKLIGHT_OFF		(0x0)

#define MAX_LCD_MSG_LEN     21

// Function Prototypes
HAL_StatusTypeDef WriteInstNibble(I2C_HandleTypeDef i2cdev, uint32_t dev_addr, uint8_t* pdata);
HAL_StatusTypeDef WriteInst(I2C_HandleTypeDef i2cdev, uint32_t dev_addr, uint8_t* pdata);
HAL_StatusTypeDef WriteDataNibble(I2C_HandleTypeDef i2cdev, uint32_t dev_addr, uint8_t* pdata);
HAL_StatusTypeDef WriteData(I2C_HandleTypeDef i2cdev, uint32_t dev_addr, uint8_t* pdata, uint32_t sz);
void              LCD_Init(uint16_t addr);

#define FUNC_TIMEOUT		5	// ms
#define INST_EXEC_DELAY     50  // us
#define NIBBLE_MASK			0xF0
#define LO_NIBBLE_SHIFT		4

static inline uint8_t GetHiNibble(uint8_t* pdata)
{
	return ((*pdata) & NIBBLE_MASK);
}

static inline uint8_t GetLoNibble(uint8_t* pdata)
{
	return ((*pdata) << LO_NIBBLE_SHIFT) & NIBBLE_MASK;
}

#endif /* INC_2004A_I2C_LCD_H_ */

