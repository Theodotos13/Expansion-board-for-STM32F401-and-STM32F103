/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Demonstration of the capabilities of the expansion board for Blue Pill and Black Pill.
  * @author    		 : Theodotos
  * @license				 : GNU GPL v3, 29 June 2007		https://www.gnu.org/licenses/why-not-lgpl.html
  * blog						 : https://dzen.ru/theodotos
  * Youtube				 :	 https://www.youtube.com/@Theodotos_
  ******************************************************************************
  * Rus:
  * Это демонстрационная програма для платы расширения к отладочным платам
  * "Blue Pill" (STM32F103C8T6) и "Black Pill" (STM32F401CCU6).
  *
  * Схема и печатная плата здесь: https://oshwlab.com/b_bikman/project_lsxkgfyc
  *
  * Программный код распространяется по лицензии GPL v3 Licens
  *
  * Eng:
  * A program demonstrating the capabilities of a power expansion board designed
  * for Blue Pill (STM32F103C8T6) and Black Pill (STM32F401CCU6) development
  * boards.
  *
  * Schematic and PCB here: https://oshwlab.com/b_bikman/project_lsxkgfyc
  *
  * The schematic and board are distributed under the MIT license.
  *
  * This software is distributed under the GPL v3 License.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ST7735.h"				// библиотека для дисплея / display library
#include "thermocouple.h"		// библиотека для термопар /thermocouple library
#include "RotaryEncoder.h"	// библиотека для энкодера с кнопкой / Library for encoder with button
#include "w25qxx.h"				// библиотека для микросхемы flash / library for a flash chip
#include "24LC512.h"				// библиотека для микросхемы EEPROM / library for an EEPROM chip
#include "stdio.h"
#include "strings.h"
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
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */

/* Структуры для работы с энкодерами (для каждого энкодера своя)
 * Structures for working with encoders (a separate one for each encoder)*/
DL_Encoder enc1;
DL_Encoder enc2;

/* здесь хранятся последние положения энкодеров.
 * The latest encoder positions are stored here. */
int16_t lastPosEnc1, lastPosEnc2;

/* В эту переменную записывается коэффициент заполнения ШИМ для вывода PB4 (разъем H12).
 * ШИМ на этом выходе реализован программный, так как к нему я подключаю жало паяльника Т12,
 * а для него необходима низкая частота, а моём случае - 50 Гц, чтобы при коэффициенте заполнения
 * 90% оставалось достаточно времени, чтобы измерить температуру жала, так как термопара в этом
 * жале расположена последовательно с нагревателем. Если быть точнее - спираль самого нагревателя
 * вместе с металлическим стержнем, который к ней припаян и есть термопара. Если вам не нужно
 * греть жало паяльника - выход PB4 можно подключить к 1-му каналу таймера 3 и реализовать на нём
 * аппаратный ШИМ с необходимой частотой.
 * The PWM duty cycle for output PB4 (connector H12) is stored in this variable. Software-based PWM
 * is used for this output because I am connecting a T12 soldering tip to it; this requires a low
 * frequency—50 Hz in my case—so that at a 90% duty cycle, there is enough time to measure the tip's
 * temperature, given that the thermocouple inside the tip is connected in series with the heating
 * element. To be more precise, the heating coil itself, along with the metal rod soldered to it,
 * acts as the thermocouple. If you do not need to heat the soldering tip, you can connect output
 * PB4 to Timer 3, Channel 1, and implement hardware PWM at the required frequency. */
volatile uint16_t P_MOSperiod;

/* Время последнего обновления дисплея. Частота обновления 100 кадров в секунду
 * Time of the last display update. Refresh rate: 100 frames per second. */
uint32_t DispUpdateTime;

/* массив термоэдс для термопары типа К в микровольтах (0 - 550 °C)
 * Array of thermo. EMF ​​for a Type K thermocouple in microvolts (0–550°C) */
extern const uint16_t __thermK[551];

/* массив сопротивления для NTC 10 кОм (0 - 50 °C)
 * resistance array for 10 kΩ NTC (0–50 °C) */
extern const uint16_t NTC10K[51];

/* В эту структуру после инициализации помещается вся информация о микросхеме w25qxx
 * After initialization, all information about W25Qxx is stored in this structure. */
extern w25qxx_t w25qxx;

/* Текущие координаты курсора на дисплее. Они обновляются библиотекой для ST7735, но
 * при необходимости их можно прочитать из программы
 * The current cursor coordinates on the display. They are updated by the ST7735 library,
 * but can also be read from the program if necessary. */
extern volatile unsigned int Tmp_x;
extern volatile unsigned int Tmp_y;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */
/* инициализация W25Qxx и вывод на экран информации о микросхеме
 * W25Qxx initialization and display of chip information */
uint8_t SPIflashInit(void);

void TurnOffAllPeripherals (ModeStruct *mode);	/* Выключение всех выходов | Turning off all outputs */
void ConfigOutputs (ModeStruct *mode);					/* Настройка всех выходов | Configuring all outputs */
void StartScreen (void);												/* Вывод статического текста на экран | Displaying static text on the screen */
void UpdateDisplay(DisplayInfo *info);					/* обновление информации на экране | screen information update */

/* Сохранение настроек всех выходов во Flash память W25Qxx
 * Saving settings for all outputs to W25Qxx Flash memory. */
void SaveDataToFlash(DisplayInfo *info, uint8_t *buff);

/* Настройка выходов в соответствии с сохранёнными во Flash памяти данными
 * Configuring outputs according to data stored in Flash memory */
void LoadDataFromFlash(DisplayInfo *info, uint8_t *buff);

/* Сохранение настроек всех выходов во EEPROM память 24LCxx
 * Saving settings for all outputs to 24LCxx EEPROM memory. */
void SaveDataToEEPROM(DisplayInfo *info, 	EEPROM_HandleTypeDef *h24lc512, uint8_t *buff);


/* Настройка выходов в соответствии с сохранёнными в EEPROM памяти данными
 * Configuring outputs according to data stored in EEPROM memory */
void LoadDataFromEEPROM(DisplayInfo *info, 	EEPROM_HandleTypeDef *h24lc512, uint8_t *buff);

/* Настройка каждого выхода по отдельности
 * Individual configuration of each output */
int8_t SetH6 (ModeStruct *mode, int16_t percent);
int8_t SetH10 (ModeStruct *mode, int16_t percent);
int8_t SetH11 (ModeStruct *mode, int16_t percent);
int8_t SetH12 (ModeStruct *mode, int16_t percent);
int8_t SetH13 (ModeStruct *mode, int16_t percent);
int8_t SetH14 (ModeStruct *mode, int16_t percent);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	/* Структуры для работы с термопарами. Первые две для работы с микросхемами MAX6675.
	 * Вторые две для измерения температуры при помощи операционного усилителя
	 * и АЦП микроконтроллера.
	 * Structures for working with thermocouples. The first two are for use with MAX6675 chips.
	 * The second two are for temperature measurement using an operational amplifier
	 * and the microcontroller's ADC.*/
	ThermocoupleStruct MAX6675_1;
	ThermocoupleStruct MAX6675_2;
	ThermocoupleStruct ThermK2;
	ThermocoupleStruct ThermK1;

	EEPROM_HandleTypeDef _24LC512;	// Структура для работы с микросхемой 24LC512 | Structure for working with the 24LC512 chip
	uint32_t now = HAL_GetTick();		// Фиксируем текущее время | We record the current time.

	/* Текущий режим (какой именно выход сейчас настраивается энкодерами)
	 * Current mode (specifically, which output is currently being configured via the encoders)*/
	uint8_t tmpMode = 0;

	/* В этой структуре хранится информация о выбранном режиме и настройках коэффициентов заполнения
	 * ШИМ для каждого выхода
	 * This structure stores information about the selected mode and PWM duty cycle settings for each output. */
	ModeStruct mode = {};

	/* В эту структуру записываются данные, которые необходимо выводить на экран
	 * Data that needs to be displayed on the screen is written to this structure. */
	DisplayInfo info = {};
	info.mode = &mode;
	info.PrintFlag = 0;

	uint8_t sBuf[64];	// буфер для чтения/записи flash | flash read/write buffer
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
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USB_DEVICE_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  /* Запуск выводов 1 и 2 таймера 2, а также 1, 2, 3 и 4 таймера 3 в режиме ШИМ
   * для управления мощностью нагрузки на выходах
   * Activating outputs 1 and 2 of Timer 2, as well as outputs 1, 2, 3, and 4 of Timer 3,
   * in PWM mode to control load power at the outputs. */
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
	/* Запуск таймера 4 в режиме прерываний по переполнению для реализации программного ШИМ с частотой
	 * 50 Гц на выходе PB4 (разъем H12)
	 * Start Timer 4 in overflow interrupt mode to implement software PWM at 50 Hz on output PB4 (connector H12) */
	HAL_TIM_Base_Start_IT(&htim4);
	/* Отключение питания на выходах всех разъёмов, к которым подключена нагрузка
	 * Cutting off power to the outputs of all connectors to which a load is connected. */
  TurnOffAllPeripherals(&mode);

  /* Настройки библиотеки thermocouple.h
   * Thermocouple.h library settings */
  /* Настройка SPI для работы с модулями MAX6675
   * Configuring SPI for Operation with MAX6675 Modules */
  SetSPI(&hspi2, SPI2);
  /* Настройка работы с модулями MAX6675 (подробное описание в файле thermocouple.h)
   * Configuring operation with MAX6675 modules (detailed description in the thermocouple.h file) */
  SetMAX6675(&MAX6675_1, MAX6675_CS1_GPIO_Port, MAX6675_CS1_Pin);
  SetMAX6675(&MAX6675_2, MAX6675_CS2_GPIO_Port, MAX6675_CS2_Pin);
  /* Настройка АЦП для измерения температуры при помощи АЦП микроконтроллера через операционные усилители
   * Configuring the ADC to measure temperature using the microcontroller's ADC via operational amplifiers. */
  SetADC(&hadc1, ADC1);
  /* Настройка внешнего источника опорного напряжения для АЦП
   * Configuring an external reference voltage source for the ADC */
  SetVRef(2.45, ADC_CHANNEL_1);
  /* Настройка измерения температуры холодного спая, а также двух каналов АЦП для двух термопар.
   * Для обоих термопар используется один и тот-же терморезистор для измерения температуры
   * холодного спая, но необходимо его инициализировать для каждой термопары отдельно.
   * Configuration of cold-junction temperature measurement and two ADC channels for two thermocouples.
   * The same thermistor is used for cold-junction temperature measurement for both thermocouples,
   * but it must be initialized separately for each thermocouple. */
  SetTermColdJunc(&ThermK1, ADC_CHANNEL_2, NTC10K, sizeof(NTC10K), 4700, NTC);
  SetTermColdJunc(&ThermK2, ADC_CHANNEL_2, NTC10K, sizeof(NTC10K), 4700, NTC);
  SetThermocouple(&ThermK1, 300, RCJ_ON, 0, ADC_CHANNEL_3, __thermK, sizeof(__thermK));
  SetThermocouple(&ThermK2, 301, RCJ_ON, 0, ADC_CHANNEL_4, __thermK, sizeof(__thermK));

  /* Настройка энкодеров. Эта функция используется, если оба контакта энкодера и
   * контакт кнопки подключены к одному порту GPIO
   * Encoders configuration. This function is used if both encoder contacts and
   * the button contact are connected to the same GPIO port. */
  RotaryEncoder(&enc1, Enc_dt1_GPIO_Port, Enc_clk1_Pin, Enc_dt1_Pin, Enc_btn1_Pin, FOUR0);
  RotaryEncoder(&enc2, Enc_dt2_GPIO_Port, Enc_clk2_Pin, Enc_dt2_Pin, Enc_btn2_Pin, FOUR0);
  /* Обнуление позиций энкодеров
   * Resetting encoder positions */
  setPosition(&enc1, 0);
	lastPosEnc1 = 0;
  setPosition(&enc2, 0);
	lastPosEnc2 = 0;

	/* Настройка дисплея, передаём функции структуру SPI_HandleTypeDef, а также порт и выводы,
	 * к отороым подключены линии CS и RS. Reset дисплея подключен к +3.3В,
	 * поэтому вместо номера его вывода передаём 0.
	 * To configure the display, we pass the SPI_HandleTypeDef structure to the function,
	 * along with the port and pins connected to the CS and RS lines.
	 * The display's reset pin is connected to +3.3V, so we pass 0 instead of a pin number. */
  LCD(&hspi2, LCD_CS_GPIO_Port, LCD_CS_Pin, LCD_RS_Pin, 0);
  /* Настраиваем смещение экрана, оно может отличаться у разных дисплеев, поэтому значения подбираем.
   * Два важных момента: 1. После изменения смещения необходимо отключать питание от дисплея,
   * или от платы полностью, иначе на экране может остаться старая заливка и изменения настроек
   * не будут  видны. 2. Настраивать смещение необходимо до инициализации и оно может быть разным
   * для разной ориентации экрана.
   * Adjust the screen offset; since this value can vary between displays, you will need to determine
   * the correct setting experimentally. Two important points: 1. After changing the offset, you must
   * disconnect power from the display or the board entirely; otherwise, the previous image content
   * may persist on the screen, making the setting changes invisible. 2. The offset must be configured
   * prior to initialization, and the value may differ depending on the screen orientation. */
  SetOffset(2, 1);
  /* Инициализация дисплея. Экраны на драйвере ST7735 бывают 128*128 и 128*160, поэтому
   * первый аргумент - соотношение сторон. Второй аргумент - тип вывода цветов: RGB или BGR.
   * Если цвета отображаются некорректно - попробуйте вместо MADCTL_RGB передать MADCTL_BGR.
   * Display initialization. ST7735-based displays come in 128x128 and 128x160 resolutions, so
   * the first argument specifies the aspect ratio. The second argument defines the color output
   * mode: RGB or BGR. If the colors appear incorrect, try passing MADCTL_BGR instead of MADCTL_RGB. */
	Init(LCD128X160, MADCTL_RGB);
	/* Настройки цвета текста, цвета фона, выбор шрифта (смотри файл "Font_8x16.h"), включение,
	 * или отключение прозрачности вывода текста
	 * Settings for text color, background color, and font selection (see the "Font_8x16.h" file),
	 * as well as enabling or disabling text output transparency. */
	SetDisplay(YELLOW, BLACK, font_8x16, TRANSPORENT_OFF);
	/* Ориентация дисплея: LANDSCAPE (альбомная), PORTRAIT (портретная) и если дисплей перевёрнут -
	 * можно выбрать LANDSCAPE_INV, или PORTRAIT_INV
	 * Display orientation: LANDSCAPE, PORTRAIT; if the display is inverted, you can select
	 *  LANDSCAPE_INV or PORTRAIT_INV. */
	SetOrientation(LANDSCAPE);

	/* Инициализация микросхемы flash памяти W25Qxx | Initialization of the W25Qxx flash memory chip*/
	if(SPIflashInit()) {		// Если всё в порядке - на экран выводится информация о микросхеме, иначе - сообщение об ошибке
		Clear();							// If order, information about the chip is displayed; otherwise, an error message appears.
		cursor(0, 32);
		SetColor(RED);
		Print("Ошибка инициализации\r\nмикросхемы flash");		// Flash chip initialization error
		HAL_Delay(5000);
	}
	W25qxx_ReadByte(sBuf, 0);		// Если во flash записаны настройки выходов - загрузить их
	if(sBuf[0] != 0xFF) {			// If output settings are stored in flash, load them
		LoadDataFromFlash(&info, sBuf);
	  setPosition(&enc1, info.Enc1);
		lastPosEnc1 = info.Enc1;
	  setPosition(&enc2, info.Enc2);
		lastPosEnc2 = info.Enc2;
	}
	if(EEPROM_Init(&_24LC512, &hi2c1)) {		// Инициализация и попытка чтения EEPROM.
		Clear();															// Если ошибка - вывести на экран сообщение EEPROM ERROR
		SetColor(RED);												// Initialization and attempt to read EEPROM.
		PrintXY("EEPROM ERROR", 20, 32);			// If an error occurs, display the message "EEPROM ERROR" on the screen.
		HAL_Delay(5000);
	} else {
		if(EEPROM_Read(&_24LC512, 0, (uint8_t*)sBuf, 18)) {	// Попытка чтения из EEPROM | Attempt to read from EEPROM
			Clear();																					// Если ошибка - вывести на экран сообщение EEPROM ERROR
			SetColor(RED);																		// If an error occurs, display the message "EEPROM ERROR" on the screen.
			PrintXY("EEPROM ERROR", 20, 32);
			HAL_Delay(5000);
		} else {																						// иначе вывести сообщение EPROM OK
			Clear();																					// otherwise, display the message "EPROM OK"
			SetColor(GREEN);
			PrintXY("EEPROM OK", 40, 32);
			HAL_Delay(500);
		}
	}
	ConfigOutputs(&mode);		// Настроить выходы. | Configure the outputs.
	StartScreen();						// Вывести на экран начальную информацию. | Display the initial information on the screen.
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		/* Измеряем температуру каждые 500мс, но чтобы программа не зависала - HAL_Delay не используем,
		 * вместо этого считываем счётчик тиков (1 раз/мс) и когда он станет на 500 больше, чем во время
		 * предыдущего измерения температуры - измеряем снова.
		 * We measure the temperature every 500 ms, but to prevent the program from freezing, we do not
		 * use HAL_Delay; instead, we read the tick counter (which updates once per millisecond) and
		 * perform the measurement again when the count exceeds the value from the previous measurement by 500. */
		if((now + 500) <= HAL_GetTick()) {
			now = HAL_GetTick();														// записываем значение счётчика тиков | we record the tick counter value
			info.T1 = Get_temperature(&MAX6675_1, MAX6675);	// измеряем температуру на левой микросхеме MAX6675 | We measure the temperature on the left MAX6675 chip.
			info.T2 = Get_temperature(&MAX6675_2, MAX6675);	// измеряем температуру на правой микросхеме MAX6675 | We measure the temperature on the right-hand MAX6675 chip.
			info.adc = GetADC(ADC_CHANNEL_5);								// Измеряем температуру на входе усилителя U5 (неинвертирующий) | We measure the temperature at the input of amplifier U5 (non-inverting).

			/* Этот усилитель настроен на измерение температуры на жале для паяльника Т12, поэтому измерение разрешается
			 * только когда на жало не подаётся напряжение питания для его нагрева
			 * This amplifier is configured to measure the temperature of the T12 soldering tip; therefore, measurements
			 * are permitted only when no heating voltage is applied to the tip. */
			if(info.adc < 2300) {													// Если на жало не подано питание - измеряем температуру. | If power is not supplied to the tip, measure the temperature.
				HAL_Delay(1);																// Задержка для стабилизации напряжения | Voltage stabilization delay
				info.T3 = Get_temperature(&ThermK2, THREM_K);
				info.Tkj = GetTempColdJunc(&ThermK2);
			}
		info.T4 = Get_temperature(&ThermK1, THREM_K);
			info.PrintFlag = 1;
		}

		/* Опрос кнопок энкодеров, начиная с левой | Polling the encoder buttons, starting with the left one. */
		if(ButtonCheck(&enc1) == 1) {						// короткое нажатие | short press
			mode.mode1++;												// переключение на следующий режим | switching to the next mode
			if(mode.mode1 == 4) {								// если режим больше третьего - возврат к нулевому | if the mode is higher than 3, return to 0
				mode.mode1 = 0;
			}
			HAL_Delay(400);											// задержка, чтобы успеть убрать палец с кнопки до повторного её опросаa | delay to allow time to remove one's finger from the button before it is polled again
		} else if(ButtonCheck(&enc1) == 2) {		// длинное нажатие | long press
			SaveDataToFlash(&info, sBuf);
			cursor(0, 80);
			SetColor(WHITE);
			Print("Сохранено во flash");				// Saved to flash memory
			HAL_Delay(1500);
			cursor(0, 80);
			SetColor(GREEN);
			Print("                  ");
		} else if(ButtonCheck(&enc1) == 3) {		// двойной щелчок | double-click
			SaveDataToEEPROM(&info, &_24LC512, sBuf);
			cursor(0, 80);
			SetColor(WHITE);
			Print("Сохранено в EEPROM");					// Saved to EEPROM memory
			HAL_Delay(1500);
			cursor(0, 80);
			SetColor(GREEN);
			Print("                  ");
		}
		/* аналогично первой кнопке проходит опрос второй
		 * The polling of the second button proceeds in the same way as that of the first. */
		if(ButtonCheck(&enc2) == 1) {
			if(mode.mode2) {
				mode.mode2 = 0;
			} else {
				mode.mode2 = 1;
			}
			HAL_Delay(400);
		} else if(ButtonCheck(&enc2) == 2) {
			LoadDataFromFlash(&info, sBuf);
			tmpMode = 0xFF;
			ConfigOutputs(&mode);
			DispUpdateTime = 0;
			UpdateDisplay(&info);
			cursor(0, 80);
			SetColor(WHITE);
			Print("Загружено из flash");			// Loaded from flash
			HAL_Delay(1500);
			cursor(0, 80);
			SetColor(GREEN);
			Print("                  ");
		} else if(ButtonCheck(&enc2) == 3) {
			LoadDataFromEEPROM(&info, &_24LC512, sBuf);
			tmpMode = 0xFF;
			ConfigOutputs(&mode);
			DispUpdateTime = 0;
			UpdateDisplay(&info);
			cursor(0, 80);
			SetColor(WHITE);
			Print("Загружено из EEPROM");			// Loaded from EEPROM
			HAL_Delay(1500);
			cursor(0, 80);
			SetColor(GREEN);
			Print("                   ");
		}
		/* Чтобы настройки коэффициента заполнения ШИМ одного выхода не переносились на другой
		 * при переключении режимов - проверяем, если режим переключен - записываем в переменную
		 * EncX значение, которое соответствует именно этому выводу
		 * To prevent the PWM duty cycle settings of one output from carrying over to the other when
		 * switching modes, we check if the mode has changed; if so, we write the value corresponding
		 * specifically to that output into the variable EncX. */
		if(tmpMode != ((mode.mode1 << 2) | mode.mode2)) {
			int16_t *ptr = &mode.H12enc;
			info.Enc1 = *(ptr + mode.mode1);
			ptr = &mode.H11enc;
			info.Enc2 = *(ptr + mode.mode2);
			setPosition(&enc1, info.Enc1);
			setPosition(&enc2, info.Enc2);
		}
		// Записываем в переменную tmpMode текущие режимы. | We save the current modes to the tmpMode variable.
		tmpMode = (mode.mode1 << 2) | mode.mode2;

		/* Считываем положение энкодеров и подаём ШИМ сигнал на выходы разъёмов H6, H10, H11, H12, H13 и H14
		 * левый энкодер управляет левыми разъёмами (H12, H6, H13, H10), а правый - правыми (H11, H14).
		 * Разъем, на который будет подан шим сигнал, определяется выбранным при помощи кнопок режимом:
		 * mode1 = 0 - H12
		 * mode1 = 1 - H6
		 * mode1 = 2 - H13
		 * mode1 = 3 - H10
		 * mode2 = 0 - H11
		 * mode2 = 1 - H14
		 * We read the encoder positions and output a PWM signal to connectors H6, H10, H11, H12, H13, and H14;
		 * the left encoder controls the left connectors (H12, H6, H13, H10), while the right encoder controls
		 * the right ones (H11, H14). The specific connector receiving the PWM signal is determined by the mode
		 * selected via the buttons:
		 * mode1 = 0 - H12
		 * mode1 = 1 - H6
		 * mode1 = 2 - H13
		 * mode1 = 3 - H10
		 * mode2 = 0 - H11
		 * mode2 = 1 - H14 */
		info.Enc1 = getPosition(&enc1);
		switch(mode.mode1) {
		case 0:
			info.Enc1 = SetH12(&mode, info.Enc1);
			break;
		case 1:
			info.Enc1 = SetH6(&mode, info.Enc1);
			break;
		case 2:
			info.Enc1 = SetH13(&mode, info.Enc1);
			break;
		case 3:
			info.Enc1 = SetH10(&mode, info.Enc1);
			break;
		}

		info.Enc2 = getPosition(&enc2);
		switch(mode.mode2) {
		case 0:
			info.Enc2 = SetH11(&mode, info.Enc2);
			break;
		case 1:
			info.Enc2 = SetH14(&mode, info.Enc2);
			break;
		}

		UpdateDisplay(&info);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 4-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 500-1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim2, TIM_CHANNEL_1);
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim2, TIM_CHANNEL_2);
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 4-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.Pulse = 500-1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim3, TIM_CHANNEL_3);
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_TIM_DISABLE_OCxPRELOAD(&htim3, TIM_CHANNEL_4);
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(W25Qxx_CS_GPIO_Port, W25Qxx_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, MAX6675_CS2_Pin|MAX6675_CS1_Pin|PB4_P_MOS_Pin|LCD_RS_Pin
                          |LCD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : W25Qxx_CS_Pin */
  GPIO_InitStruct.Pin = W25Qxx_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(W25Qxx_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PC14 PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Enc_btn2_Pin Enc_dt2_Pin Enc_clk2_Pin Enc_btn1_Pin
                           Enc_dt1_Pin Enc_clk1_Pin */
  GPIO_InitStruct.Pin = Enc_btn2_Pin|Enc_dt2_Pin|Enc_clk2_Pin|Enc_btn1_Pin
                          |Enc_dt1_Pin|Enc_clk1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : MAX6675_CS2_Pin MAX6675_CS1_Pin PB4_P_MOS_Pin LCD_RS_Pin
                           LCD_CS_Pin */
  GPIO_InitStruct.Pin = MAX6675_CS2_Pin|MAX6675_CS1_Pin|PB4_P_MOS_Pin|LCD_RS_Pin
                          |LCD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SW3_Pin */
  GPIO_InitStruct.Pin = SW3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SW3_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
int8_t SetH6 (ModeStruct *mode, int16_t percent)
{
	if(percent != lastPosEnc1) {
		if(percent > 100) {
			percent = 100;
		} else if (percent < 10) {
			if(lastPosEnc1 > percent) {
				percent = 0;
			} else {
				percent = 10;
			}
		}
		setPosition(&enc1, percent);
		mode->H6enc = percent;
		lastPosEnc1 = percent;
		percent > 0? (TIM3->CCR2 = percent * 10): (TIM3->CCR2 = 0);
	}
	return percent;
}

int8_t SetH10 (ModeStruct *mode, int16_t percent)
{
	if(percent != lastPosEnc1) {
		if(percent > 100) {
			percent = 100;
		} else if (percent < 10) {
			if(lastPosEnc1 > percent) {
				percent = 0;
			} else {
				percent = 10;
			}
		}
		setPosition(&enc1, percent);
		mode->H10enc = percent;
		lastPosEnc1 = percent;
		percent > 0? (TIM2->CCR2 = percent * 10): (TIM2->CCR2 = 0);
	}
	return percent;
}

int8_t SetH11 (ModeStruct *mode, int16_t percent)
{
	if(percent != lastPosEnc2) {
		if(percent > 100) {
			percent = 100;
		} else if (percent < 10) {
			if(lastPosEnc2 > percent) {
				percent = 0;
			} else {
				percent = 10;
			}
		}
		setPosition(&enc2, percent);
		mode->H11enc = percent;
		lastPosEnc2 = percent;
		percent > 0? (TIM3->CCR3 = percent * 10): (TIM3->CCR3 = 0);
	}
	return percent;
}

int8_t SetH12 (ModeStruct *mode, int16_t percent)
{
	if(percent != lastPosEnc1) {
			if(percent > 100) {
				percent = 100;
			} else if (percent < 10) {
				if(lastPosEnc1 > percent) {
					percent = 0;
				} else {
					percent = 10;
				}
			}
			setPosition(&enc1, percent);
			mode->H12enc = percent;
			lastPosEnc1 = percent;
			percent > 0? (P_MOSperiod = (100 - percent)): (P_MOSperiod = 101);
		}
		return percent;
}

int8_t SetH13 (ModeStruct *mode, int16_t percent)
{
	if(percent != lastPosEnc1) {
		if(percent > 100) {
			percent = 100;
		} else if (percent < 10) {
			if(lastPosEnc1 > percent) {
				percent = 0;
			} else {
				percent = 10;
			}
		}
		setPosition(&enc1, percent);
		mode->H13enc = percent;
		lastPosEnc1 = percent;
		percent > 0? (TIM2->CCR1 = percent * 10): (TIM2->CCR1 = 0);
	}
	return percent;
}

int8_t SetH14 (ModeStruct *mode, int16_t percent)
{
	if(percent != lastPosEnc2) {
		if(percent > 100) {
			percent = 100;
		} else if (percent < 10) {
			if(lastPosEnc2 > percent) {
				percent = 0;
			} else {
				percent = 10;
			}
		}
		setPosition(&enc2, percent);
		mode->H14enc = percent;
		lastPosEnc2 = percent;
		percent > 0? (TIM3->CCR4 = percent * 10 - 1): (TIM3->CCR4 = 0);
	}
	return percent;
}

void StartScreen (void)
{
	SetColor(YELLOW);
	Clear();
	PrintXY("Привет!\r\nMAX6675_1 =\r\nMAX6675_2 =\r\n", 32, 0);
	Print("T12=      TCJ=     \r\nThermK=");
	PrintXY("Duty=    Duty=    \r\nOutL=    OutR=", 0, 94);
	SetColor(GREEN);
}

void UpdateDisplay(DisplayInfo *info)
{
	if(info->PrintFlag == 1) {
		PrintIntXY(info->T1, 96, 16);								// выводим значение температуры левой микросхемы
		Print("°C");															 	// знак градусов Цельсия
		while(Tmp_x < 152) Print(" ");							// заполняем оставшуюся часть строки пробелами для стирания предыдущего значения
		PrintIntXY(info->T2, 96, 32);								//выводим значение температуры правой микросхемы
		Print("°C");																// знак градусов Цельсия
		while(Tmp_x < 152) Print(" ");							// заполняем оставшуюся часть строки пробелами для стирания предыдущего значения
		Print("\r\n");															// переставляем курсор в начало следующей строки

		PrintIntXY(info->T3, 32,48);								// Вывод на дисплей температуры
		Print("°C");															 	// знак градусов Цельсия
		while(Tmp_x < 56)Print(" ");								// Заполнение пробелами части строки до следующего значения
		PrintIntXY(info->Tkj, 112,48);							// Вывод температуры холодного спая
		Print("°C");															 	// знак градусов Цельсия
		while(Tmp_x < 104)Print(" ");								// Заполнение пробелами части строки до следующего значения
		PrintIntXY(info->T4, 56, 64);								// Выводим температуру с термопары типа К
		Print("°C");															 	// знак градусов Цельсия
		while(Tmp_x < 152) Print(" ");							// Заполнение пробелами части строки до следующего значения
//		PrintIntXY(info->ADC, 120, 48);							// Вывод значения АЦП
//		while(Tmp_x < 152) Print(" ");							// Заполнение пробелами оставшейся части строки
		info->PrintFlag = 0;
	}
	/* Обновляем экран с частотой 100 кадров в секунду. Чаще не вижу смысла */
	if((DispUpdateTime + 10) < HAL_GetTick()) {
		/* Выводим на экран положение энкодеров и выбранные при помощи кнопок режимы */
		PrintIntXY(info->Enc1, 40, 94);
		while(Tmp_x < 72) Print(" ");
		PrintIntXY(info->Enc2, 112, 94);
		while(Tmp_x < 152) Print(" ");

		switch(info->mode->mode1) {
		case 0:
			PrintXY("H12", 40, 110);
			break;
		case 1:
			PrintXY("H6 ", 40, 110);
			break;
		case 2:
			PrintXY("H13", 40, 110);
			break;
		case 3:
			PrintXY("H10", 40, 110);
			break;
		}
		switch(info->mode->mode2) {
		case 0:
			PrintXY("H11", 112, 110);
			break;
		case 1:
			PrintXY("H14 ", 112, 110);
			break;
		}
		DispUpdateTime = HAL_GetTick();
	}
}

uint8_t SPIflashInit(void)
{
	int16_t pos1 = getPosition(&enc1);
	int16_t pos2 = getPosition(&enc2);

	Clear();
	W25qxx_Init();
	if(w25qxx.ID == 0) {
		return 1;
	}
	cursor(0, 0);
	SetColor(GREEN);
	switch(w25qxx.ID) {
	case W25Q10:
		Print("   W25Q10 \r\nID:");
		break;
	case W25Q20:
		Print("   W25Q20 \r\nID:");
		break;
	case W25Q40:
		Print("   W25Q40 \r\nID:");
		break;
	case W25Q80:
		Print("   W25Q80 \r\nID:");
		break;
	case W25Q16:
		Print("   W25Q16 \r\nID:");
		break;
	case W25Q32:
		Print("   W25Q32 \r\nID:");
		break;
	case W25Q64:
		Print("   W25Q64 \r\nID:");
		break;
	case W25Q128:
		Print("   W25Q128 \r\nID:");
		break;
	case W25Q256:
		Print("   W25Q256 \r\nID:");
		break;
	case W25Q512:
		Print("   W25Q512 \r\nID:");
		break;
	}
	char str[127];
	sprintf(str, "%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X\r\n", w25qxx.UniqID[0],\
																											w25qxx.UniqID[1],\
																											w25qxx.UniqID[2],\
																											w25qxx.UniqID[3],\
																											w25qxx.UniqID[4],\
																											w25qxx.UniqID[5],\
																											w25qxx.UniqID[6],\
																											w25qxx.UniqID[7]);
	SetColor(WHITE);
	Print(str);
	SetColor(YELLOW);
	Print("Memorie:     ");
	SetColor(WHITE);
	PrintInt(w25qxx.CapacityInKiloByte);
	SetColor(YELLOW);
	Print("kB\r\nPageSize:    ");
	SetColor(WHITE);
  	PrintInt(w25qxx.PageSize);
  	SetColor(YELLOW);
  	Print("\r\nPageCount:   ");
  	SetColor(WHITE);
	PrintInt(w25qxx.PageCount);
	SetColor(YELLOW);
	Print("\r\nSectorSize:  ");
	SetColor(WHITE);
	PrintInt(w25qxx.SectorSize);
	SetColor(YELLOW);
	Print("\r\nSectorCount: ");
	SetColor(WHITE);
	PrintInt(w25qxx.SectorCount);
	while(1) {
		if(ButtonCheck(&enc1)) break;
		if(ButtonCheck(&enc2)) break;
		if(getPosition(&enc1) != pos1) break;
		if(getPosition(&enc2) != pos2) break;
	}
	if(ButtonCheck(&enc1)) {
		while(ButtonCheck(&enc1));
	}
	if(ButtonCheck(&enc2)) {
		while(ButtonCheck(&enc2));
	}
	pos1 = getPosition(&enc1);
	pos2 = getPosition(&enc2);
	fill_color_area(0, 159, 32, 127, BLACK);
	cursor(0, 32);
	SetColor(YELLOW);
	Print("BlockSize:  ");
	SetColor(WHITE);
	PrintInt(w25qxx.BlockSize);
	SetColor(YELLOW);
	Print("\r\nBlockCount: ");
	SetColor(WHITE);
	PrintInt(w25qxx.BlockCount);
	while(1) {
		if(ButtonCheck(&enc1))break;
		if(ButtonCheck(&enc2))break;
		if(getPosition(&enc1) != pos1)break;
		if(pos2 != getPosition(&enc2))break;
	}
	setPosition(&enc1, 0);
	setPosition(&enc2, 0);
	return 0;
}

void SaveDataToFlash(DisplayInfo *info, uint8_t *buff)
{
	W25qxx_EraseSector(0);
	HAL_Delay(15);
	buff[0] = info->Enc1 >> 8;
	buff[1] = info->Enc1 & 0xFF;
	buff[2] = info->Enc2 >> 8;
	buff[3] = info->Enc2 & 0xFF;
	buff[4] = info->mode->H12enc >> 8;
	buff[5] = info->mode->H12enc & 0xFF;
	buff[6] = info->mode->H6enc >> 8;
	buff[7] = info->mode->H6enc & 0xFF;
	buff[8] = info->mode->H13enc >> 8;
	buff[9] = info->mode->H13enc & 0xFF;
	buff[10] = info->mode->H10enc >> 8;
	buff[11] = info->mode->H10enc & 0xFF;
	buff[12] = info->mode->H11enc >> 8;
	buff[13] = info->mode->H11enc & 0xFF;
	buff[14] = info->mode->H14enc >> 8;
	buff[15] = info->mode->H14enc & 0xFF;
	buff[16] = info->mode->mode1;
	buff[17] = info->mode->mode2;
	W25qxx_WriteSector(buff, 0, 0, 18);
}

void LoadDataFromFlash(DisplayInfo *info, uint8_t *buff)
{
	W25qxx_ReadPage(buff, 0, 0, 18);
	info->Enc1 = (int16_t)(buff[0] << 8) | buff[1];
	info->Enc2 = (int16_t)(buff[2] << 8) | buff[3];
	info->mode->H12enc = (int16_t)(buff[4] << 8) | buff[5];
	info->mode->H6enc = (int16_t)(buff[6] << 8) | buff[7];
	info->mode->H13enc = (int16_t)(buff[8] << 8) | buff[9];
	info->mode->H10enc = (int16_t)(buff[10] << 8) | buff[11];
	info->mode->H11enc = (int16_t)(buff[12] << 8) | buff[13];
	info->mode->H14enc = (int16_t)(buff[14] << 8) | buff[15];
	info->mode->mode1 = buff[16];
	info->mode->mode2 = buff[17];
}


void SaveDataToEEPROM(DisplayInfo *info, 	EEPROM_HandleTypeDef *h24lc512, uint8_t *buff)
{
	EEPROM_Erase(h24lc512, 0, 18, 0xFF);
	HAL_Delay(15);
	buff[0] = info->Enc1 >> 8;
	buff[1] = info->Enc1 & 0xFF;
	buff[2] = info->Enc2 >> 8;
	buff[3] = info->Enc2 & 0xFF;
	buff[4] = info->mode->H12enc >> 8;
	buff[5] = info->mode->H12enc & 0xFF;
	buff[6] = info->mode->H6enc >> 8;
	buff[7] = info->mode->H6enc & 0xFF;
	buff[8] = info->mode->H13enc >> 8;
	buff[9] = info->mode->H13enc & 0xFF;
	buff[10] = info->mode->H10enc >> 8;
	buff[11] = info->mode->H10enc & 0xFF;
	buff[12] = info->mode->H11enc >> 8;
	buff[13] = info->mode->H11enc & 0xFF;
	buff[14] = info->mode->H14enc >> 8;
	buff[15] = info->mode->H14enc & 0xFF;
	buff[16] = info->mode->mode1;
	buff[17] = info->mode->mode2;
	EEPROM_Write(h24lc512, 0, buff, 18);
}

void LoadDataFromEEPROM(DisplayInfo *info, 	EEPROM_HandleTypeDef *h24lc512, uint8_t *buff)
{
	EEPROM_Read(h24lc512, 0, buff, 18);
	info->Enc1 = (int16_t)(buff[0] << 8) | buff[1];
	info->Enc2 = (int16_t)(buff[2] << 8) | buff[3];
	info->mode->H12enc = (int16_t)(buff[4] << 8) | buff[5];
	info->mode->H6enc = (int16_t)(buff[6] << 8) | buff[7];
	info->mode->H13enc = (int16_t)(buff[8] << 8) | buff[9];
	info->mode->H10enc = (int16_t)(buff[10] << 8) | buff[11];
	info->mode->H11enc = (int16_t)(buff[12] << 8) | buff[13];
	info->mode->H14enc = (int16_t)(buff[14] << 8) | buff[15];
	info->mode->mode1 = buff[16];
	info->mode->mode2 = buff[17];
}

void TurnOffAllPeripherals (ModeStruct *mode)
{
	lastPosEnc1 = 10;
	SetH6(mode, 0);
	lastPosEnc1 = 10;
	SetH10(mode, 0);
	lastPosEnc2 = 10;
	SetH11(mode, 0);
	lastPosEnc1 = 10;
	SetH12(mode, 0);
	lastPosEnc1 = 10;
	SetH13(mode, 0);
	lastPosEnc2 = 10;
	SetH14(mode, 0);
	HAL_GPIO_WritePin(W25Qxx_CS_GPIO_Port, W25Qxx_CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MAX6675_CS1_GPIO_Port, MAX6675_CS1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MAX6675_CS2_GPIO_Port, MAX6675_CS2_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

void ConfigOutputs (ModeStruct *mode)
{
	lastPosEnc1 = 110;
	SetH6(mode, mode->H6enc);
	lastPosEnc1 = 110;
	SetH10(mode, mode->H10enc);
	lastPosEnc2 = 110;
	SetH11(mode, mode->H11enc);
	lastPosEnc1 = 110;
	SetH12(mode, mode->H12enc);
	lastPosEnc1 = 110;
	SetH13(mode, mode->H13enc);
	lastPosEnc2 = 110;
	SetH14(mode, mode->H14enc);

	switch(mode->mode1) {
	case 0:
		setPosition(&enc1, mode->H12enc);
		break;
	case 1:
		setPosition(&enc1, mode->H6enc);
		break;
	case 2:
		setPosition(&enc1, mode->H13enc);
		break;
	case 3:
		setPosition(&enc1, mode->H10enc);
		break;
	}
	switch(mode->mode2) {
	case 0:
		setPosition(&enc2, mode->H11enc);
		break;
	case 1:
		setPosition(&enc2, mode->H14enc);
		break;
	}
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
