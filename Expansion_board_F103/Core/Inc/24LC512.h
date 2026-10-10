/*
 * 24LC512.h
 *
 *  Created on: Jan 9, 2025
 *      Author: Darth
 *
 *  This file incorporates code from STM32-24LC512-EEPROM
 *  (https://github.com/DarthFelus/STM32-24LC512-EEPROM/),
 *  licensed under the MIT License.
 *  Copyright (c) 2025 Darth
 *
 */

#ifndef INC_24LC512_H_
#define INC_24LC512_H_

#ifdef __cplusplus
extern "C" {
#endif

/************************************************ INCLUDES ************************************************/

#include "main.h"

/************************************************ DEFINES ************************************************/

/* Device parameters */
#define EEPROM_ADDR 0xA0						//Base I2C address (0xA0 when A0-A2 pins are grounded)
#define EEPROM_PAGE_SIZE 128					//128 bytes per page for 24LC512
#define EEPROM_MEMORY_SIZE 65536				//Total memory size 64KB (512Kbit)
#define EEPROM_ADDR_SIZE I2C_MEMADD_SIZE_16BIT	//16-bit addressing mode


/************************************************ TYPES ************************************************/

/* EEPROM handle structure */
typedef struct
{
	I2C_HandleTypeDef *hi2c;					//Pointer to HAL I2C handle
	uint8_t device_addr;						//I2C device address
	uint32_t timeout;							//Timeout for operations in milliseconds
} EEPROM_HandleTypeDef;

/* Status codes */
typedef enum
{
	EEPROM_OK,									//Operation completed successfully
	EEPROM_BUSY,								//Device is busy with previous operation
	EEPROM_ERROR,								//General error occurred
	EEPROM_TIMEOUT								//Operation timed out
} EEPROM_StatusTypeDef;



/************************************************ FUNCTION PROTOTYPES ************************************************/
EEPROM_StatusTypeDef EEPROM_Init(EEPROM_HandleTypeDef *handle, I2C_HandleTypeDef *hi2c);
EEPROM_StatusTypeDef EEPROM_Write(EEPROM_HandleTypeDef *handle, uint16_t address, uint8_t *data, uint16_t size);
EEPROM_StatusTypeDef EEPROM_Read(EEPROM_HandleTypeDef *handle, uint16_t address, uint8_t *data, uint16_t size);
EEPROM_StatusTypeDef EEPROM_Erase(EEPROM_HandleTypeDef *handle, uint16_t address, uint16_t size, uint8_t eraseValue);

#ifdef __cplusplus
}
#endif


#endif /* INC_24LC512_H_ */
