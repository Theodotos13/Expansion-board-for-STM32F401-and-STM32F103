/*
 * thermocouple.h
 *
 * Библиотека для работы с термопарой (прямое измерение при помощи АЦП микроконтроллера и
 * с использованием микросхемы MAX6675), а также с термисторами PTC и NTC
 *
 * Библиотека распространяется на условиях лицензии MIT
 * https://mit-license.org/
 *
 * A library for working with thermocouples (direct measurement using the microcontroller's
 * ADC and the MAX6675 chip), as well as with PTC and NTC thermistors.
 *
 * This work is licensed under a MIT License
 * https://mit-license.org/
 *
 *  Created on: 	03.09.2026
 *      Author: 	Theodotos
 *      YouTube
 *     channel:	www.youtube.com/@Theodotos_
 *      		blog:	www.dzen.ru/theodotos
 *      	 email:	sanichbogdan@yandex.ru
 */

#ifndef _THERMOCOUPLE_H
#define _THERMOCOUPLE_H

#include "main.h"

#ifdef		HAL_ADC_MODULE_ENABLED
/* Макросы для удобной настройки */
/* Macros for convenient configuration */
#define RCJ_ON		0	/* Включено реальное измерение температуры холодного спая / Real-time cold-junction temperature measurement is included. */
#define RCJ_OFF	1	/* Отключено реальное измерение температуры холодного спая / Real-time cold-junction temperature measurement is disabled. */
#define NTC			0	/* Термистор NTC / NTC thermistor */
#define PTC			1	/* Термистор PTC / PTC thermistor */
#endif

/* Способы измерения температуры */
/* Methods of temperature measurement */
typedef enum {
	THREM_K = 1,	/* Термопара типа К и АЦП / Type K thermocouple and ADC */
	MAX6675,			/* С помощью микросхемы MAX6675 / Using the MAX6675 chip */
	THERMISTOR		/* С помощью NTC или PTC термистора / Using NTC or PTC thermistor */
}THERM_TYPE;

/* Структура для настройки метода измерения */
/* Structure for configuring the measurement method */
typedef struct {
		GPIO_TypeDef  			*GPIOSPICS;				/* порт GPIO (GPIOA, GPIOB,...)/ GPIO port  (GPIOA, GPIOB,...) */
		uint16_t						PINSPICS;					/* пин GPIO (GPIO_PIN_0,...GPIO_PIN_15) / GPIO pin (GPIO_PIN_0,...GPIO_PIN_15) */
#ifdef		HAL_ADC_MODULE_ENABLED
		uint32_t 					ADCchannel;				/* канал АЦП (ADC_CHANNEL_0, ADC_CHANNEL_1,…)/ ADC channel (ADC_CHANNEL_0, ADC_CHANNEL_1,…) */
		uint32_t 					ADC_Zero_channel;	/* канал АЦП датчика температуры хол. спая  / cold junction temperature sensor ADC channel */
		uint16_t 					ZeroADC;						/* компенсация смещения нуля на выходе ОУ / compensation of the op-amp output zero offset */
		float 							gain; 							/* коэффициент усиления / gain coefficient */
		const uint16_t 		*TERMTABLE;				/* указатель на массив с таблицей напряжений термопары / pointer to array thermocouple voltage table */
		const uint16_t 		*R1TABLE;					/* указатель на массив с таблицей сопротивлений термистора / pointer to an array containing the thermistor resistance table */
		uint16_t 					SizeR1table;				/* размер массива с таблицей сопротивлений / size of the resistance table array */
		uint16_t 					SizeTable;					/* размер массива с таблицей напряжений термопары / size of the thermocouple voltage table array */
		uint32_t						__R2;							/* Сопротивление резистора R2 для NTC и PTC термисторов / Resistance of R2 for NTC and PTC thermistors */
		uint8_t						thermistor;				/* Тип термистора (NTC, или PTC) / Thermistor type (NTC, or PTC) */
		uint8_t						thermistorCJ;			/* Тип термистора хол. спая (NTC, или PTC) / Cold junction thermistor type (NTC or PTC) */
		uint32_t						R2;								/* Сопротивление R2 для измерения темп. хол. спая / Resistor R2 for cold junction temperature measurement */
		uint8_t						RCJ;								/* Вкл./Выкл. измерение темп. хол. спая (RCJ_ON/RCJ_OFF) / On/Off cold junc. temp. measurement (RCJ_ON/RCJ_OFF) */
		uint16_t 					TempColdJunc;			/* температура холодного спая / cold junction temperature */
#endif
		}ThermocoupleStruct;

/* Структура для хранения выбранных модулей АЦП и SPI */
/* Structure for storing selected ADC and SPI modules */
typedef struct {
#ifdef HAL_SPI_MODULE_ENABLED
		SPI_HandleTypeDef 	hspiX;							/* Указатель на структуру для настройки SPI / Pointer to the SPI configuration structure */
#endif
#ifdef HAL_ADC_MODULE_ENABLED
		ADC_HandleTypeDef 	hadcX;							/* Указатель на структуру для настройки АЦП / Pointer to the ADC configuration structure */
#endif
#ifdef HAL_SPI_MODULE_ENABLED
		SPI_TypeDef      	*SPIinstance;			/* SPI1, SPI2 or SPI3 */
	#ifdef __STM32F1xx_HAL_H
			uint32_t CR1_tmp;
	#endif
#endif
#ifdef		HAL_ADC_MODULE_ENABLED
		ADC_TypeDef      	*ADCinstance;			/* ADC1, SPI2 or ADC3*/
#endif
}SPIADCStruct;

#ifdef		HAL_ADC_MODULE_ENABLED
/* Функция для инициализации АЦП */
/* Function for ADC initialization */
void SetADC(ADC_HandleTypeDef *hadc, ADC_TypeDef  *ADCinstance);
#endif

#ifdef HAL_SPI_MODULE_ENABLED
/* Функция для инициализации SPI */
/* Function for SPI initialization */
void SetSPI(SPI_HandleTypeDef *HSPI, SPI_TypeDef *SPIX);
#endif

#ifdef		HAL_ADC_MODULE_ENABLED
/* Настройка измерения температуры с помощью термопары и АЦП микроконтроллера */
/* Setup of temperature measurement using a thermocouple and a microcontroller ADC */
void SetThermocouple (ThermocoupleStruct * therm, float Gain, uint16_t ColdJunc, uint16_t ZADC,\
																	uint32_t channel1, const uint16_t *table, uint16_t SizeTable);

/* Настройка измерения температуры при помощи термистора */
/* Configuring temperature measurement using a thermistor */
void SetThermistor (ThermocoupleStruct * therm, 	const uint16_t *R1table, uint16_t SizeTable,\
																							uint32_t r2, uint8_t ntcptc, uint32_t channel1);
#endif

#ifdef HAL_SPI_MODULE_ENABLED
/* Настройка измерения температуры с помощью микросхемы MAX6675 */
/* Configuring temperature measurement using the MAX6675 chip */
void SetMAX6675(ThermocoupleStruct * therm, GPIO_TypeDef *gpioCS, uint16_t CS_pin);
#endif

#ifdef		HAL_ADC_MODULE_ENABLED
/* Настройка измерения температуры холодного спая */
/* Setting up cold junction temperature measurement */
void SetTermColdJunc (ThermocoupleStruct * therm, uint32_t channel2, const uint16_t *R1table, \
																							uint16_t SizeTable, uint32_t r2, uint8_t ntcptc);
#endif

/* Функция измеряет температуру и возвращает ее значение в градусах Цельсия */
/* The function measures the temperature and returns its value in degrees Celsius */
uint16_t Get_temperature(ThermocoupleStruct * therm, THERM_TYPE type);

#ifdef		HAL_ADC_MODULE_ENABLED
/* Функция измеряет температуру холодного спая и возвращает значение в градусах Цельсия */
/* The function measures the cold-junction temperature and returns the value in degrees Celsius*/
uint8_t GetTempColdJunc (ThermocoupleStruct * therm);

/* Функция измеряет напряжение на указанном входе АЦП */
/* The function measures the voltage at the specified ADC input */
uint16_t GetADC(uint32_t Channel);

/* Настройка внешнего источника опорного напряжения */
/* Configuration of an external reference voltage source */
void SetVRef(float Uref, uint32_t Channel);
#endif

#endif /* APPLICATION_USER_CORE_THERMOCOUPLE_H_ */
