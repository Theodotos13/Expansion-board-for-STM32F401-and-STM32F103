/*
 * 24LC512.c
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

/*
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Library for using in STM32L4 series microcontroller
 *
 * Microchip
 * 24LC512
 * 512Kb I2C compatible 2-wire Serial EEPROM
 *
 * Datasheet link: https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/24AA512-24LC512-24FC512-512-Kbit-I2C-Serial-EEPROM-DS20001754.pdf
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */



#include "24LC512.h"
#include <string.h>

static EEPROM_StatusTypeDef EEPROM_WaitForWriteComplete(EEPROM_HandleTypeDef *handle);

/* Initialize EEPROM handle */
EEPROM_StatusTypeDef EEPROM_Init(EEPROM_HandleTypeDef *handle, I2C_HandleTypeDef *hi2c)
{
	if (handle == NULL || hi2c == NULL) return EEPROM_ERROR;

	handle->hi2c = hi2c;
	handle->device_addr = EEPROM_ADDR;
	handle->timeout = 1000; // Default timeout in ms

	return EEPROM_OK;
}


/*
 * Wait for write operation to complete using ACK polling
 * 		@handle: EEPROM handle pointer
 * 		@return EEPROM_OK if successful, EEPROM_TIMEOUT if timeout occurs
 */
static EEPROM_StatusTypeDef EEPROM_WaitForWriteComplete(EEPROM_HandleTypeDef *handle)
{
    uint32_t StartTick = HAL_GetTick();
    uint8_t dummy = 0;
    // Poll with a zero-length read until ACK received
    while (HAL_I2C_Mem_Read(handle->hi2c, handle->device_addr, 0, EEPROM_ADDR_SIZE,
                           &dummy, 1, 1) != HAL_OK)
    {
        if ((HAL_GetTick() - StartTick) > handle->timeout) return EEPROM_TIMEOUT;

        HAL_Delay(1);
    }

    return EEPROM_OK;
}

/*
 * Write function for both single byte and buffer operations
 * 		@handle: EEPROM handle pointer
 * 		@address: Starting address in EEPROM
 * 		@data: Pointer to data buffer (can be single byte or multiple bytes)
 * 		@size: Number of bytes to write (1 for single byte)
 * 		@return EEPROM status
 */
EEPROM_StatusTypeDef EEPROM_Write(EEPROM_HandleTypeDef *handle, uint16_t address, uint8_t *data, uint16_t size)
{

    if (address + size >= EEPROM_MEMORY_SIZE || size == 0 || data == NULL || handle == NULL) return EEPROM_ERROR;

    uint16_t bytes_left = size;
    uint16_t current_address = address;
    uint8_t *data_ptr = data;

    if (bytes_left > 0)
    {
    	// Calculate current page write size
    	uint16_t PageOffset = current_address % EEPROM_PAGE_SIZE;;
    	uint16_t currentSize = (bytes_left > (EEPROM_PAGE_SIZE - PageOffset)) ? (EEPROM_PAGE_SIZE - PageOffset) : bytes_left;
    	// Write current page
    	if (HAL_I2C_Mem_Write(handle->hi2c, handle->device_addr,
    			current_address, EEPROM_ADDR_SIZE,
				data_ptr, currentSize, handle->timeout) != HAL_OK) return EEPROM_ERROR;
    	// Wait for write to complete
        EEPROM_StatusTypeDef status = EEPROM_WaitForWriteComplete(handle);
        if (status != EEPROM_OK) return status;
        // Update pointers and counters
    	current_address += currentSize;
    	data_ptr += currentSize;
    	bytes_left -= currentSize;
    }

    return EEPROM_OK;
}

/*
 * Read function for both single byte and buffer operations
 * 		@handle: EEPROM handle pointer
 * 		@address: Starting address in EEPROM
 * 		@data: Pointer to data buffer where read data will be stored
 * 		@size: Number of bytes to read (1 for single byte)
 * 		@return EEPROM status
 */
EEPROM_StatusTypeDef EEPROM_Read(EEPROM_HandleTypeDef *handle, uint16_t address, uint8_t *data, uint16_t size)
{
	if (address + size >= EEPROM_MEMORY_SIZE || size == 0 || data == NULL || handle == NULL) return EEPROM_ERROR;

	if (HAL_I2C_Mem_Read(handle->hi2c, handle->device_addr, address, EEPROM_ADDR_SIZE, data, size, handle->timeout) != HAL_OK) return EEPROM_ERROR;

	return EEPROM_OK;

}


/*
 * Erase memory region by writing specified value (default 0xFF)
 * 		@handle: EEPROM handle pointer
 * 		@address: Starting address to erase
 * 		@size: Number of bytes to erase
 * 		@eraseValue: Value to write (typically 0xFF)
 * 		@return EEPROM status
 */

EEPROM_StatusTypeDef EEPROM_Erase(EEPROM_HandleTypeDef *handle, uint16_t address, uint16_t size, uint8_t eraseValue)
{
    // Parameter validation
    if (handle == NULL || (address + size) > EEPROM_MEMORY_SIZE || size == 0) {
        return EEPROM_ERROR;
    }

    uint8_t erasePage[EEPROM_PAGE_SIZE];
    memset(erasePage, eraseValue, EEPROM_PAGE_SIZE);

    uint16_t bytesLeft = size;
    uint16_t currentAddr = address;

    while (bytesLeft > 0) {
        uint16_t pageOffset = currentAddr % EEPROM_PAGE_SIZE;
        uint16_t currentSize = (bytesLeft > (EEPROM_PAGE_SIZE - pageOffset)) ?
                              (EEPROM_PAGE_SIZE - pageOffset) : bytesLeft;

        if (HAL_I2C_Mem_Write(handle->hi2c, handle->device_addr,
                             currentAddr, EEPROM_ADDR_SIZE,
                             erasePage, currentSize,
                             handle->timeout) != HAL_OK) {
            return EEPROM_ERROR;
        }

        if (EEPROM_WaitForWriteComplete(handle) != EEPROM_OK) {
            return EEPROM_ERROR;
        }

        currentAddr += currentSize;
        bytesLeft -= currentSize;
    }

    return EEPROM_OK;
}
