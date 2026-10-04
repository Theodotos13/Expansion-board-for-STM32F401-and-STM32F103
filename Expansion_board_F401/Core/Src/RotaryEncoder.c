// -----
// RotaryEncoder.cpp - Library for using rotary encoders.
// This class is implemented for use with the Arduino environment.
//
// Copyright (c) by Matthias Hertel, http://www.mathertel.de
//
// The library was ported for STM32 by Theodotos 09.29.2026
// blog: 	https://dzen.ru/theodotos
// Youtube:	https://www.youtube.com/@Theodotos_
//
// This work is licensed under a BSD 3-Clause style license,
// https://www.mathertel.de/License.aspx.
//
// More information on: http://www.mathertel.de/Arduino
// -----
// Changelog: see RotaryEncoder.h
// -----

#include "RotaryEncoder.h"

#define LATCH0 0  // input state at position 0
#define LATCH3 3  // input state at position 3


// The array holds the values �1 for the entries where a position was decremented,
// a 1 for the entries where the position was incremented
// and 0 in all the other (no change or not valid) cases.

const int8_t KNOBDIR[] = {
  0, -1, 1, 0,
  1, 0, 0, -1,
  -1, 0, 0, 1,
  0, 1, -1, 0
};

// positions: [3] 1 0 2 [3] 1 0 2 [3]
// [3] is the positions where my rotary switch detends
// ==> right, count up
// <== left,  count down


// ----- Initialization and Default Values -----

/**
 * @brief Default constructor that initializes the RotaryEncoder with non-hardware specific setup.
 *
 * This constructor creates a RotaryEncoder instance with only software initialization.
 * No pins are configured or reserved. This is useful for scenarios where:
 * - Pins will be managed externally
 * - Hardware-specific tick() will be called with explicit pin values
 * - Deferred or dynamic pin configuration is needed
 *
 * @param mode The latch mode defining the encoder sensitivity.
 *   See RotaryEncoder.h for details on the available modes.
 *
 * Initialization sets:
 * - Encoder mode
 * - Position counter to 0
 * - Internal state variables to initial values (no motion detected)
 * - No pins are reserved (_pin1 and _pin2 set to NO_PIN)
 * - Timestamp tracking for rotation speed calculation
 *
 * @note To use this constructor effectively, call tick(sig1, sig2) with explicit pin values
 *       in your main loop, or use the two-parameter constructor if you need automatic pin handling.
 *
 * @see RotaryEncoder(int pin1, int pin2, LatchMode mode) for hardware-managed pin setup
 * @see tick(int sig1, int sig2) for software-managed pin input
 */
 
void RotaryEncoderDiffPorts(DL_Encoder *encoder, GPIO_TypeDef *port_clk, uint16_t pin_clk,\
																								GPIO_TypeDef *port_dat, uint16_t pin_data,\
																								GPIO_TypeDef *port_sw, uint16_t pin_sw, LatchMode mode)
{
	 encoder->sig1 = 0;
	 encoder->sig2 = 0;
	 encoder->_mode = mode;

	 encoder->_oldState = encoder->sig1 | (encoder->sig2 << 1);

	 //Порт подключения энкодера
	 encoder->port_dat = port_dat;
	 encoder->port_clk = port_clk;
	 encoder->port_sw = port_sw;

	 encoder->pin_clk = pin_clk;  //Пин подключения clk
	 encoder->pin_data = pin_data;  //Пин подключения data
	 encoder->pin_sw = pin_sw;  //Пин подключения кнопки

	 encoder->last_clk_state = HAL_GPIO_ReadPin(port_clk, pin_clk);  //Предыдущее состояние пина clk
	 encoder->last_sw_state = HAL_GPIO_ReadPin(port_sw, pin_sw);  //Предыдущее состояние пина кнопки

	 encoder->timer_frot = 0;  //Программный таймер для событий быстрых поворотов
	 encoder->counter_frot = 0;  //Счетчик переключений при быстром повороте

	 encoder->timer_dbc = 0;  //Программный таймер для события двойного клика

	 encoder->timer_hold = 0;  //Программный таймер для события удержания кнопки
	 encoder->hold_flag = 0;  //Флаг для события удержания кнопки

	 encoder->pos = 0;  //Счетчик позиции равен нулю

	 encoder->ButtonPressed = 0;
	 encoder->ButtonPressTime = 0;
	 encoder->ButtonReleaseTime = 0;
	 encoder->ButtonPressDuration = 0;
	 encoder->ButtonReleaseDuration = 0;
	 encoder->ButtonPressType = 0;

	 // start with position 0;
	 encoder->_position = 0;
	 encoder->_oldState = 0;
	 encoder->_positionExtPrev = encoder->_positionExt = 0;
	 encoder->_positionExtTimePrev = encoder->_positionExtTime = HAL_GetTick();

	 // when not started in motion, the current state of the encoder should be 3
	 encoder->sig1 = HAL_GPIO_ReadPin(encoder->port_clk, encoder->pin_clk);
	 encoder->sig2 = HAL_GPIO_ReadPin(encoder->port_dat, encoder->pin_data);
	 encoder->_oldState = encoder->sig1 | (encoder->sig2 << 1);
}

void RotaryEncoder(DL_Encoder *encoder, GPIO_TypeDef *port, uint16_t pin_clk, uint16_t pin_data, uint16_t pin_sw, LatchMode mode)  //Инициализатор
{
	RotaryEncoderDiffPorts(encoder, port, pin_clk, port, pin_data, port, pin_sw, mode);
}
 
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
 *
 * Hardware Setup:
 * - Configures both pins as INPUT_PULLUP for reliable signal detection
 * - Reads the initial state of pin1 and pin2 using digitalRead()
 * - Establishes the initial position based on current pin values
 * - Stores pin numbers for use with the non-parameterized tick() method
 *
 * Interrupt-Safe Usage:
 * This constructor is suitable for both polling and interrupt-driven modes:
 * - For polling: call tick() periodically in your main loop
 * - For interrupts: attach interrupt handlers to these pins and call tick(sig1, sig2)
 *                   from the interrupt handler with explicit pin values for better performance
 *
 * Initial State:
 * - Position counter initialized to 0
 * - Internal state variables set based on reading the actual pin values
 * - Timestamp tracking initialized for rotation speed calculation
 *
 * @note If both pins are negative or not in the valid range [0, MAX_PIN], the hardware
 *       setup is skipped but the encoder still initializes with software defaults.
 * @note The pins must support INPUT_PULLUP mode on your microcontroller.
 * @note For maximum interrupt responsiveness, consider using the parameterized tick(sig1, sig2)
 *       variant and reading pins directly in your interrupt handler.
 */
//void RotaryEncoder(DL_Encoder *encoder) {
//
//}  // RotaryEncoder()


long getPosition(DL_Encoder *encoder) {
  return encoder->_positionExt;
}  // getPosition()


Direction getDirection(DL_Encoder *encoder) {
  Direction ret = NOROTATION;

  if (encoder->_positionExtPrev > encoder->_positionExt) {
    ret = COUNTERCLOCKWISE;
    encoder->_positionExtPrev = encoder->_positionExt;
  } else if (encoder->_positionExtPrev < encoder->_positionExt) {
    ret = CLOCKWISE;
    encoder->_positionExtPrev = encoder->_positionExt;
  } else {
    ret = NOROTATION;
    encoder->_positionExtPrev = encoder->_positionExt;
  }

  return ret;
}


void setPosition(DL_Encoder *encoder, long newPosition) {
  switch (encoder->_mode) {
    case FOUR3:
    case FOUR0:
      // only adjust the external part of the position.
    	encoder->_position = ((newPosition << 2) | (encoder->_position & 0x03L));
    	encoder->_positionExt = newPosition;
    	encoder->_positionExtPrev = newPosition;
      break;

    case TWO03:
      // only adjust the external part of the position.
    	encoder->_position = ((newPosition << 1) | (encoder->_position & 0x01L));
    	encoder->_positionExt = newPosition;
    	encoder->_positionExtPrev = newPosition;
      break;
  }  // switch

}  // setPosition()


// Slow, but Simple Variant by directly Read-Out of the Digital State within loop-call
void tick(DL_Encoder *encoder) {
	encoder->sig1 = HAL_GPIO_ReadPin(encoder->port_clk, encoder->pin_clk);
	encoder->sig2 = HAL_GPIO_ReadPin(encoder->port_dat, encoder->pin_data);
  tick_(encoder);
}  // tick()


// When a faster method than digitalRead is available you can _tick with the 2 values directly.
void tick_(DL_Encoder *encoder) {
  unsigned long now = HAL_GetTick();
  int8_t thisState = encoder->sig1 | (encoder->sig2 << 1);

  if (encoder->_oldState != thisState) {
  	encoder->_position += KNOBDIR[thisState | (encoder->_oldState << 2)];
  	encoder->_oldState = thisState;

    switch (encoder->_mode) {
      case FOUR3:
        if (thisState == LATCH3) {
          // The hardware has 4 steps with a latch on the input state 3
        	encoder->_positionExt = encoder->_position >> 2;
        	encoder->_positionExtTimePrev = encoder->_positionExtTime;
        	encoder->_positionExtTime = now;
        }
        break;

      case FOUR0:
        if (thisState == LATCH0) {
          // The hardware has 4 steps with a latch on the input state 0
        	encoder->_positionExt = encoder->_position >> 2;
        	encoder->_positionExtTimePrev = encoder->_positionExtTime;
        	encoder->_positionExtTime = now;
        }
        break;

      case TWO03:
        if ((thisState == LATCH0) || (thisState == LATCH3)) {
          // The hardware has 2 steps with a latch on the input state 0 and 3
        	encoder->_positionExt = encoder->_position >> 1;
        	encoder->_positionExtTimePrev = encoder->_positionExtTime;
        	encoder->_positionExtTime = now;
        }
        break;
    }  // switch
  }  // if
  // Button polling
  // Если кнпку только нажали - фиксируем время и поднимаем флаг "ButtonPressed"
  if ((HAL_GPIO_ReadPin(encoder->port_sw, encoder->pin_sw) == LOW) && (encoder->ButtonPressed == 0)) {
  		encoder->ButtonPressed = 1;									// поднимаем флаг "ButtonPressed"
  		encoder->ButtonPressTime = HAL_GetTick();		// фиксируем время нажатия кнопки
  		encoder->ButtonPressType = 0;								// сбрасываем тип нажатия кнопки
  		if(encoder->ButtonReleaseDuration < 120) {	// Если после прошлого нажатия прошло меньше 120мс
  			encoder->ButtonPressType = 3;							// "double-click"
  		}
  		encoder->ButtonPressDuration = 0;						// сбрасываем время удерживания кнопки нажатой
  		encoder->ButtonReleaseDuration = 0;					// сбрасываем время, в течении которого кнопка отпущена
  }

  // Если кнопку только отпустили - фиксируем время и сбрасываем флаг "ButtonPressed"
  if ((HAL_GPIO_ReadPin(encoder->port_sw, encoder->pin_sw) == HIGH) && (encoder->ButtonPressed == 1)) {
		encoder->ButtonPressed = 0;									// сбрасываем флаг "ButtonPressed"
		encoder->ButtonReleaseTime = HAL_GetTick();	// фиксируем время отпускания кнопки

		// Рассчитываем время удержания кнопки
		encoder->ButtonPressDuration = encoder->ButtonReleaseTime - encoder->ButtonPressTime;
  }

  // Если кнопка отпущена - считаем время до следующего нажатия
	if(HAL_GPIO_ReadPin(encoder->port_sw, encoder->pin_sw) == HIGH) {
		encoder->ButtonReleaseDuration = HAL_GetTick() - encoder->ButtonReleaseTime;
	}

  // Если кнопка удержана дольше 1 сек - устанавливаем тип нажатия "удерживание"
  if ((HAL_GPIO_ReadPin(encoder->port_sw, encoder->pin_sw) == LOW) && (HAL_GetTick() > (encoder->ButtonPressTime + 1000))) {
  		encoder->ButtonPressType = 2;		// holding
  }

  // Если время удерживания кнопки нажатой больше 10мс и меньше 1с, а также в течении 120мс
  // после того, как ее отпустили, не было повторного нажатия - устанавливаем тип "короткое нажатие"
  if ((encoder->ButtonPressDuration > 10) && (encoder->ButtonPressDuration < 1000)) {

  		// Если после прошлого нажатия кнопки прошло более 120мс и не был зафиксирован "double-click"
  		if((encoder->ButtonReleaseDuration > 120) && (encoder->ButtonPressType != 3)) {
  			encoder->ButtonPressType = 1;									// short single press
  		}
  } else if (encoder->ButtonPressDuration > 1000) {	// если дольше 1с - "удерживание"
  		encoder->ButtonPressType = 2;										// holding
  }

  // Если кнопка в отпущенном состоянии дольше 0.5с - устанавливаем тип "кнопка не нажата"
  if ((HAL_GPIO_ReadPin(encoder->port_sw, encoder->pin_sw) == HIGH) && (HAL_GetTick() > (encoder->ButtonReleaseTime + 500))) {
  		encoder->ButtonPressType = 0;
  }
  // end button polling
}  // tick()


unsigned long getMillisBetweenRotations(DL_Encoder *encoder) {
  return (encoder->_positionExtTime - encoder->_positionExtTimePrev);
}

static unsigned long max(unsigned long a, unsigned long b)
{
  if(a > b) {
      return a;
  } else {
      return b;
  }
}

unsigned long getRPM(DL_Encoder *encoder) {
  // calculate max of difference in time between last position changes or last change and now.
  unsigned long timeBetweenLastPositions = encoder->_positionExtTime - encoder->_positionExtTimePrev;
  unsigned long timeToLastPosition = HAL_GetTick() - encoder->_positionExtTime;
  unsigned long t = max(timeBetweenLastPositions, timeToLastPosition);
  return 60000.0 / ((float)(t * 20));
}

/*void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

}*/


/* Обработка кнопки энкодера. При коротком нажатии функция возвращает 1, при длинном - 2,
при двойном клике - 3. Если кнопка не была нажата - функция возвращает 0 */
uint8_t ButtonCheck(DL_Encoder *encoder)
{
	return encoder->ButtonPressType;
}

// End
