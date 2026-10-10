// -----
// RotaryEncoder.h - Library for using rotary encoders.
// This class is implemented for use with the Arduino environment.
//
// Copyright (c) by Matthias Hertel, http://www.mathertel.de
//
// This work is licensed under a BSD 3-Clause style license,
// https://www.mathertel.de/License.aspx.
//
// More information on: http://www.mathertel.de/Arduino
// -----
// 18.01.2014 created by Matthias Hertel
// 16.06.2019 pin initialization using INPUT_PULLUP
// 10.11.2020 Added the ability to obtain the encoder RPM
// 29.01.2021 Options for using rotary encoders with 2 state changes per latch.
// 06.06.2024 Implementation of tick() with passing the input values for more performant implementations.
// 21.02.2025 Documentation and Constructor without hardware initialization added.
// -----
//
// The library was ported for STM32 by Theodotos 09.29.2026
// blog: 	https://dzen.ru/theodotos
// Youtube:	https://www.youtube.com/@Theodotos_
//
// This library is best used with a timer interrupt; this allows the encoders and buttons to be polled 
// at intervals determined by the timer, independently of the main program. You can then call functions 
// getPosition(&enc1) and ButtonCheck(&enc1) from the main program to retrieve the polling results. 
// To do this—without needing to start a separate timer—include the header file "RotaryEncoder.h" 
// in the file "stm32fxxx_it.c", declare an external structure "xtern DL_Encoder enc1", and insert 
// a call to the function SysTick_Handler() into the function. tick(&enc1). 
// It will look something like this:
//
//	/* Private includes ----------------------------------------------------------*/
//	/* USER CODE BEGIN Includes */
//	#include "RotaryEncoder.h"
//	/* USER CODE END Includes */
//
//	/* Private variables ---------------------------------------------------------*/
//	/* USER CODE BEGIN PV */
//	extern DL_Encoder enc1;  //Структура энкодера
//	/* USER CODE END PV */
//
//		void SysTick_Handler(void)
//		{
//  		/* USER CODE BEGIN SysTick_IRQn 0 */
//			tick(&enc1);
//  		/* USER CODE END SysTick_IRQn 0 */
//  		HAL_IncTick();
//  		/* USER CODE BEGIN SysTick_IRQn 1 */
//
// 		/* USER CODE END SysTick_IRQn 1 */
//		}



#ifndef RotaryEncoder_h
#define RotaryEncoder_h

#include "main.h"
#include "stdbool.h"
#ifndef NO_PIN
#define NO_PIN -1
#endif

#define HIGH 0x1
#define LOW  0x0



// typedef enum { false, true } bool;


typedef enum {
  PINCLK = 0,
  PINDATA,
  SWITCH
}encISR;

typedef enum {
    NOROTATION = 0,
    CLOCKWISE = 1,
    COUNTERCLOCKWISE = -1
  }Direction;

typedef enum {
    FOUR3 = 1,  // 4 steps, Latch at position 3 only (compatible to older versions)
    FOUR0 = 2,  // 4 steps, Latch at position 0 (reverse wirings)
    TWO03 = 3   // 2 steps, Latch at position 0 and 3
  }LatchMode;


  typedef struct  //Структура с данными энкодера
  {
    	GPIO_TypeDef *port_dat;
    	GPIO_TypeDef *port_clk;
    	GPIO_TypeDef *port_sw;

  	uint16_t pin_clk;  //Пин подключения clk
  	uint16_t pin_data;  //Пин подключения data
  	uint16_t pin_sw;  //Пин подключения кнопки

  	uint8_t last_clk_state : 1;  //Последнее состояние пина clk
  	uint8_t last_sw_state : 1;  //Последнее состояние пина кнопки
  	uint8_t hold_flag : 1;  //Флаг удержания кнопки

  	int16_t counter_frot;  //Счетчик быстрых поворотов

  	uint32_t timer_frot;  //Таймер быстрых поворотов
	uint32_t timer_dbc;  //Таймер двойного клика
	uint32_t timer_hold;  //Таймер удержания кнопки

  	int32_t pos;  //Позиция энкодера
  	LatchMode _mode;  // Latch mode from initialization

  	volatile int _oldState;

  	volatile long _position;         // Internal position (4 times _positionExt)
  	volatile long _positionExt;      // External position
  	volatile long _positionExtPrev;  // External position (used only for direction checking)

  	unsigned long _positionExtTime;      // The time the last position change was detected.
  	unsigned long _positionExtTimePrev;  // The time the previous position change was detected.

    int sig1;
    int sig2;

   bool ButtonPressed;
   uint32_t ButtonPressTime;
   uint32_t ButtonReleaseTime;
   uint32_t ButtonPressDuration;
   uint32_t ButtonReleaseDuration;
   uint8_t ButtonPressType;
  } DL_Encoder;


  /**
   * @brief Constructor that initializes the RotaryEncoder with hardware pin setup.
   *
   * This constructor creates a RotaryEncoder instance with full default hardware initialization.
   * It configures the specified pins, enables internal pull-up resistors, and reads their
   * current state to establish the initial encoder position.
   *
   * @param pin1 First encoder pin (typically pin A). Use a value 0 or greater for a valid pin.
   *             A negative value or NO_PIN will skip hardware configuration.
   * @param pin2 Second encoder pin (typically pin B). Use a value 0 or greater for a valid pin.
   *             A negative value or NO_PIN will skip hardware configuration.
   * @param mode The latch mode defining the encoder sensitivity.
   *   See RotaryEncoder.h for details on the available modes.
   */
//
//  // Constructor that initializes the RotaryEncoder without hardware setup.
  void RotaryEncoder(DL_Encoder *encoder, GPIO_TypeDef *port, uint16_t pin_clk, uint16_t pin_data,\
  																																uint16_t pin_sw, LatchMode mode);  //�������������

		// функция для инициализации энкодера, контакты которого подключены к выводам разных портов GPIO
  void RotaryEncoderDiffPorts(DL_Encoder *encoder, GPIO_TypeDef *port_clk, uint16_t pin_clk,\
  																								GPIO_TypeDef *port_dat, uint16_t pin_data,\
  																								GPIO_TypeDef *port_sw, uint16_t pin_sw, LatchMode mode);
  // retrieve the current position
  long getPosition(DL_Encoder *encoder);

  // simple retrieve of the direction the knob was rotated last time. 0 = No rotation, 1 = Clockwise, -1 = Counter Clockwise
  Direction getDirection(DL_Encoder *encoder);

  // adjust the current position
  void setPosition(DL_Encoder *encoder, long newPosition);

  // call this function every some milliseconds or by using an interrupt for handling state changes of the rotary encoder.
  // This method uses the standard Arduino digitalRead() function with the 2 pins provided in the class creation.
  void tick(DL_Encoder *encoder);

  // Use this tick variant when a faster method than digitalRead is available and provide the values directly.
  // The 2 pins provided in the class creation are ignored.
  void tick_(DL_Encoder *encoder);

  // Returns the time in milliseconds between the current observed
  unsigned long getMillisBetweenRotations(DL_Encoder *encoder);

  // Returns the RPM
  unsigned long getRPM(DL_Encoder *encoder);

  uint8_t ButtonCheck(DL_Encoder *encoder);

#endif

// End
