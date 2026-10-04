/*
 * thermocouple.c
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

#include "thermocouple.h"

#ifdef		HAL_ADC_MODULE_ENABLED
/* PT100 (Ом*100)
 * Данный массив содержит таблицу сопротивлений в зависимости от температуры для
 * PTC термистора PT100 (от 0 до +350 градусов). Шаг 1 градус по Цельсию.
 * Значения Ом*100. Если Вы не используете PTC термистор – закомментируйте этот
 * массив для экономии памяти.
 * ----------------------------------------------------------------------------------------
 * This array contains a resistance-vs-temperature table for a PT100 PTC thermistor,
 * with 1°C increments. The values are in Ohms multiplied by 100 (covering the range
 * from 0°C to +350°C). If you are not using a PTC thermistor, comment out
 * this array to save memory.
 *  */
const uint16_t PT100x100[351] = {
	10390,	10429,	10468,	10507,	10546,	10585,	10624,	10663,	10701,	10740,		// 10 - 19
	10779,	10818,	10857,	10895,10934,	10973,	11012,	11051,	11089,	11128,		// 20 - 29
	11167,	11206,	11245,	11283,	11322,	11361,	11400,	11438,	11477,	11515,		// 30 - 39
	11554,	11593,	11631,	11670,	11708,	11747,	11786,	11824,	11863,	11901,		// 40 - 49
	11940,	11978,	12017,	12055,	12094,	12132,	12170,	12209,	12247,	12286,		// 50 - 59
	12324,	12362,	12401,	12439,	12478,	12516,	12554,	12593,	12631,	12670,		// 60 - 69
	12708,	12746,	12784,	12823,	12861,	12899,	12937,	12975,	13014,	13052,		// 70 - 79
	13090,	13128,	13166,	13204,	13242,	13280,	13318,	13356,	13395,	13433,		// 80 - 89
	13471,	13509,	13547,	13585,	13623,	13661,	13699,	13737,	13775,	13813,		// 90 - 99
	13851,	13889,	13927,	13964,	14002,	14040,	14078,	14116,	14153,	14191,		// 100 - 109
	14229,	14267,	14305,	14342,	14380,	14418,	14456,	14494,	14531,	14569,		// 110 - 119
	14607,	14645,	14682,	14720,	14757,	14795,	14833,	14870,	14908,	14945,		// 120 - 129
	14983,	15021,	15058,	15096,	15133,	15171,	15208,	15246,	15283,	15321,		// 130 - 139
	15358,	15396,	15433,	15471,	15508,	15546,	15583,	15621,	15658,	15696,		// 140 - 149
	15733,	15770,	15807,	15845,	15882,	15919,	15956,	15993,	16031,	16068,		// 150 - 159
	16105,	16142,	16179,	16217,	16254,	16291,	16328,	16365,	16403,	16440,		// 160 - 169
	16477,	16514,	16551,	16589,	16626,	16663,	16700,	16737,	16774,	16811,		// 170 - 179
	16848,	16885,	16922,	16959,	16996,	17033,	17070,	17107,	17143,	17180,		// 180 - 189
	17217,	17254,	17291,	17328,	17365,	17402,	17439,	17476,	17512,	17549,		// 190 - 199
	17586,	17623,	17659,	17696,	17732,	17769,	17806,	17843,	17879,	17916,		// 200 - 209
	17953,	17990,	18026,	18063,	18099,	18136,	18173,	18209,	18246,	18282,		// 210 - 219
	18319,	18355,	18392,	18428,	18465,	18501,	18538,	18574,	18611,	18647,		// 220 - 229
	18684,	18720,	18757,	18793,	18830,	18866,	18902,	18938,	18975,	19011,		// 230 - 239
	19047,	19083,	19120,	19156,	19193,	19229,	19265,	19301,	19338,	19374,		// 240 - 249
	19410,	19446,	19482,	19519,	19555,	19591,	19627,	19663,	19699,	19735,		// 250 - 259
	19771,	19807,	19843,	19879,	19915,	19951,	19987,	20023,	20059,	20095,		// 260 - 269
	20131,	20167,	20203,	20239,	20275,	20311,	20347,	20383,	20418,	20454,		// 270 - 279
	20490,	20526,	20562,	20598,	20634,	20670,	20706,	20741,	20777,	20812,		// 280 - 289
	20848,	20884,	20920,	20955,	20991,	21027,	21063,	21098,	21134,	21169,		// 290 - 299
	21205,	21241,	21276,	21312,	21347,	21383,	21419,	21454,	21490,	21525,		// 300 - 309
	21561,	21596,	21632,	21667,	21703,	21738,	21773,	21809,	21844,	21880,		// 310 - 319
	21915,	21950,	21986,	22021,	22057,	22092,	22127,	22162,	22198,	22233,		// 320 - 329
	22268,	22303,	22339,	22374,	22410,	22445,	22480,	22515,	22551,	22586,		// 330 - 339
	22621,	22656,	22691,	22726,	22761,	22796,	22831,	22866,	22902,	22937,		// 340 - 349
	22972,																													// 350
};

/* Термопара типа К / Type K Thermocouple
 * Данный массив содержит таблицу термо-ЭДС для термопары типа К. Напряжение указано
 * в микровольтах. Таблица для температуры от 0 до +550 градусов Цельсия.
 * Если вы используете микросхему MAX6675 – эта таблица не нужна, можно её закомментировать
 * для экономии памяти. Если измеряете температуру при помощи АЦП микроконтроллера – нужна,
 * при необходимости можно добавить в неё значения термо-ЭДС для других значений температуры.
 * Шаг 1 градус по Цельсию
 *
 * This array contains a thermoelectric EMF table for a Type K thermocouple. The voltage
 * is specified in microvolts. The table covers the temperature range from 0 to +550 degrees Celsius.
 * If you are using the MAX6675 chip, this table is not required and can be commented out
 * to save memory. However, if you are measuring temperature using a microcontroller's ADC,
 * the table is necessary; you can add thermoelectric EMF values for other temperatures if needed.
 * The step size is 1 degree Celsius
 * */
const uint16_t __thermK[551] = {
	0, 39, 79,	119,	158,	198,	238,	277,	317,	357,													// 0 - 9
	397, 437,	477,	517,	557,	597,	637,	677,	718,	758,											// 10 - 19
	798,	838,	879,	919,	960,	1000,	1041,	1081,	1122,	1163,							// 20 - 29
	1203,	1244,	1285,	1326,	1366,	1407,	1448,	1489,	1530,	1571,		// 30 - 39
	1612,	1653,	1694,	1735,	1776,	1817,	1858,	1899,	1941,	1982,		// 40 - 49
	2023,	2064,	2106,	2147,	2188,	2230,	2271,	2312,	2354,	2395,		// 50 - 59
	2436,	2478,	2519,	2561,	2602,	2644,	2685,	2727,	2768,	2810,		// 60 - 69
	2851,	2893,	2934,	2976,	3017,	3059,	3100,	3142,	3184,	3225,		// 70 - 79
	3267,	3308,	3350,	3391,	3433,	3474,	3516,	3557,	3599,	3640,		// 80 - 89
	3682, 3723,	3765,	3806,	3848,	3889,	3931,	3972,	4013,	4055,		// 90 - 99
	4096,	4138,	4179,	4220,	4262,	4303,	4344,	4385,	4427,	4468,		// 100 - 109
	4509, 4550,	4591,	4633,	4674,	4715,	4756,	4797, 4838,	4879,		// 110 - 119
	4920,	4961,	5002,	5043,	5084,	5124,	5165,	5206,	5247,	5288,		// 120 - 129
	5328,	5369,	5410,	5450,	5491,	5532,	5572,	5613,	5653,	5694,		// 130 - 139
	5735,	5775,	5815,	5856,	5896,	5937,	5977,	6017,	6058,	6098,		// 140 - 149
	6138,	6179,	6219,	6259,	6299,	6339,	6380,	6420,	6460,	6500,		// 150 - 159
	6540,	6580,	6620,	6660,	6701,	6741,	6781,	6821,	6861,	6901,		// 160 - 169
	6941,	6981,	7021,	7060,	7100,	7140,	7180,	7220,	7260,	7300,		// 170 - 179
	7340,	7380,	7420,	7460,	7500,	7540,	7579,	7619,	7659,	7699,		// 180 - 189
	7739,	7779,	7819,	7859,	7899,	7939,	7979,	8019,	8059,	8099,		// 190 - 199
	8138,	8178,	8218,	8258,	8298,	8338,	8378,	8418,	8458,	8499,		// 200 - 209
	8539,	8579,	8619,	8659,	8699,	8739,	8779,	8819,	8860,	8900,		// 210 - 219
	8940,	8980,	9020,	9061,	9101,	9141,	9181,	9222,	9262,	9302,		// 220 - 229
	9343,	9383,	9423,	9464,	9504,	9545,	9585,	9626,	9666,	9707,		// 230 - 239
	9747,	9788,	9828,	9869,	9909,	9950,	9991,	10031,	10072,	10113,		// 240 - 249
	10153,	10194,	10235,	10276,	10316,	10357,	10398,	10439,	10480,	10520,		// 250 - 259
	10561,	10602,	10643,	10684,	10725,	10766,	10807,	10848,	10889,	10930,		// 260 - 269
	10971,	11012,	11053,	11094,	11135,	11176,	11217,	11259,	11300,	11341,		// 270 - 279
	11382,	11423,	11465,	11506,	11547,	11588,	11630,	11671,	11712,	11753,		// 280 - 289
	11795,	11836,	11877,	11919,	11960,	12001,	12043,	12084,	12126,	12167,		// 290 - 299
	12209,	12250,	12291,	12333,	12374,	12416,	12457,	12499,	12540,	12582,		// 300 - 309
	12624,	12665,	12707,	12748,	12790,	12831,	12873,	12915,	12956,	12998,		// 310 - 319
	13040,	13081,	13123,	13165,	13206,	13248,	13290,	13331,	13373,	13415,		// 320 - 329
	13457,	13498,	13540,	13582,	13624,	13665,	13707,	13749,	13791,	13833,		// 330 - 339
	13874,	13916,	13958,	14000,	14042,	14084,	14126,	14167,	14209,	14251,		// 340 - 349
	14293,	14335,	14377,	14419,	14461,	14503,	14545,	14587,	14629,	14671,		// 350 - 359
	14713,	14755,	14797,	14839,	14881,	14923,	14965,	15007,	15049,	15091,		// 360 - 369
	15133,	15175,	15217,	15259,	15301,	15343,	15385,	15427,	15469,	15511,		// 370 - 379
	15554,	15596,	15638,	15680,	15722,	15764,	15806,	15849,	15891,	15933,		// 380 - 389
	15975,	16017,	16059,	16102,	16144,	16186,	16228,	16270,	16313,	16355,		// 390 - 399
	16397,	16439,	16482,	16524,	16566,	16608,	16651,	16693,	16735,	16778,		// 400 - 409
	16820,	16862,	16904,	16947,	16989,	17031,	17074,	17116,	17158,	17201,		// 410 - 419
	17243,	17285,	17328,	17370,	17413,	17455,	17497,	17540,	17582,	17624,		// 420 - 429
	17667,	17709,	17752,	17794,	17837,	17879,	17921,	17964,	18006,	18049,		// 430 - 439
	18091,	18134,	18176,	18218,	18261,	18303,	18346,	18388,	18431,	18473,		// 440 - 449
	18516,	18558,	18601,	18643,	18686,	18728,	18771,	18813,	18856,	18898,		// 450 - 459
	18941,	18983,	19026,	19068,	19111,	19154,	19196,	19239,	19281,	19324,		// 460 - 469
	19366,	19409,	19451,	19494,	19537,	19579,	19622,	19664,	19707,	19750,		// 470 - 479
	19792,	19835,	19877,	19920,	19962,	20005,	20048,	20090,	20133,	20175,		// 480 - 489
	20218,	20261,	20303,	20346,	20389,	20431,	20474,	20516,	20559,	20602,		// 490 - 499
	20644,	20687,	20730,	20772,	20815,	20857,	20900,	20943,	20985,	21028,		// 500 - 509
	21071,	21113,	21156,	21199,	21241,	21284,	21326,	21369,	21412,	21454,		// 510 - 519
	21497,	21540,	21582,	21625,	21668,	21710,	21753,	21796,	21838,	21881,		// 520 - 529
	21924,	21966,	22009,	22052,	22094,	22137,	22179,	22222,	22265,	22307,		// 530 - 539
	22350,	22393,	22435,	22478,	22521,	22563,	22606,	22649,	22691,	22734,		// 540 - 549
	22776,																													// 550
};

/* NTC термистор 10 кОм / NTC thermistor 10 kOhm
 * Данный массив содержит таблицу сопротивлений в зависимости от температуры для
 * NTC термистора 10 кОм. Шаг 1 градус по Цельсию. Так-как данный термистор я планирую
 * использовать только для компенсации температуры холодного спая при измерении
 * температуры термопарой – значения сопротивлений указаны только от 0 до 50 градусов
 * по Цельсию. При необходимости можно расширить таблицу, или закомментировать,
 * если NTC термистор не используется.
 *
 * This array contains a temperature-resistance lookup table for a 10 kΩ NTC thermistor,
 * with a step of 1 degree Celsius. Since I plan to use this thermistor solely for
 * cold-junction temperature compensation when measuring temperature with a thermocouple,
 * the resistance values ​​cover only the 0 to 50°C range. The table can be extended if necessary,
 * or commented out if the NTC thermistor is not being used.
 *  */
const uint16_t NTC10K[51] = {
		32742,	31113,	29576,	28123,	26750,	25451,	24223,	23062,	21963,	20922,		// 0 - 9
		19936,	19003,	18118,	17280,	16485,	15731,	15016,	14338,	13693,	13082,		// 10 - 19
		12501,	11949,	11424,	10925,	10451,	10000,	9571,	9163,	8774,	8404,		// 20 - 29
		8052,	7715,	7395,	7090,	6800,	6522,	6258,	6005,	5765,	5535,		// 30 - 39
		5315,	5105,	4905,	4714,	4531,	4356,	4189,	4029,	3876,	3729,		// 40 - 49
		3589,																													// 50
};

float Vmax = 3.3;						// Напряжение на входе VDDA / VDDA input voltage
#endif

static SPIADCStruct spiadc;		// структура для хранения настроек АЦП и SPI / structure for storing ADC and SPI settings

/**
  * @brief Это внутренняя функция библиотеки, она осуществляет инициализацию АЦП.
  * 				Обращаться из программы к ней не нужно. Используйте функцию
  * 				SetADC(ADC_HandleTypeDef *hadc, ADC_TypeDef  *ADCinstance) /
  * 				This is an internal library function; it implements the ADC setup.
  * 				You do not need to call it directly from your program.
  * 				Use the `SetADC(ADC_HandleTypeDef *hadc, ADC_TypeDef *ADCinstance)` function instead.
  *
  * @param None
  * @retval None
  * @note Если вы используете микроконтроллеры STM32F1xx или STM32F4xx – функция будет работать,
  * 				если другие – выдаст ошибку: "ADC settings are missing in void ADC_Init(void)".
  * 				Чтобы функция работала с другими микроконтроллерами – необходимо перед #else добавить
  * 				#elif defined(__STM32FXXXX_HAL_H) с именем заголовочного файла библиотеки HAL к вашему контроллеру.
  * 				После этого в файле майн открыть функцию static void MX_ADCx_Init(void), скопировать из
  * 				неё Common config. Копируем только заполнение структуры. Переход к функции HAL_ADC_Init(&hadcX) не копируем.
  * 				Затем название структуры «hadcX» во всех строках меняем на «spiadc.hadcX».
  * 				Затем в строке «spiadc.hadcX.Instance = ADCX» ADCX меняем на spiadc.ADCinstance.
  * 				После внесения этих изменений функция должна работать с вашим контроллером. /
  * 				If you are using STM32F1xx or STM32F4xx microcontrollers, the function will work; otherwise,
  * 				it will throw the error: "ADC settings are missing in void ADC_Init(void)". To make the function
  * 				work with other microcontrollers, you need to add `#elif defined(__STM32FXXXX_HAL_H)`—using
  * 				the name of your controller's HAL library header file—before the `#else` line. Next, open the
  * 				`static void MX_ADCx_Init(void)` function in your `main` file and copy the "Common config" section;
  * 				copy only the structure initialization code, not the call to `HAL_ADC_Init(&hadcX)`.
  * 				Then, replace the structure name `hadcX` with `spiadc.hadcX` in every line. Finally, in the line
  * 				`spiadc.hadcX.Instance = ADCX`, change `ADCX` to `spiadc.ADCinstance`. Once these changes are made,
  * 				the function should work with your controller.
  */
#ifdef		HAL_ADC_MODULE_ENABLED
static void ADC_Init(void)
{
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
#ifdef __STM32F4xx_HAL_H
	spiadc.hadcX.Instance = spiadc.ADCinstance;
	spiadc.hadcX.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	spiadc.hadcX.Init.Resolution = ADC_RESOLUTION_12B;
	spiadc.hadcX.Init.ScanConvMode = DISABLE;
	spiadc.hadcX.Init.ContinuousConvMode = DISABLE;
	spiadc.hadcX.Init.DiscontinuousConvMode = DISABLE;
	spiadc.hadcX.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	spiadc.hadcX.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	spiadc.hadcX.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	spiadc.hadcX.Init.NbrOfConversion = 1;
	spiadc.hadcX.Init.DMAContinuousRequests = DISABLE;
	spiadc.hadcX.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
#elif defined(__STM32F1xx_HAL_H)
  spiadc.hadcX.Instance = spiadc.ADCinstance;
  spiadc.hadcX.Init.ScanConvMode = ADC_SCAN_DISABLE;
  spiadc.hadcX.Init.ContinuousConvMode = DISABLE;
  spiadc.hadcX.Init.DiscontinuousConvMode = DISABLE;
  spiadc.hadcX.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  spiadc.hadcX.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	spiadc.hadcX.Init.NbrOfConversion = 1;
#else
#error "ADC settings are missing in void ADC_Init(void)"
	// смотри note в описании функции
	// See note in the function description.
#endif
	HAL_ADC_Init(&spiadc.hadcX);

}
#endif

/**
  * @brief Это внутренняя функция библиотеки, она осуществляет инициализацию SPI.
  * 				Обращаться из программы к ней не нужно. Используйте функцию
  * 				SetSPI(SPI_HandleTypeDef *HSPI, SPI_TypeDef *SPIX) /
  * 				This is an internal library function; it implements the SPI setup.
  * 				You do not need to call it directly from your program.
  * 				Use the `SetSPI(SPI_HandleTypeDef *HSPI, SPI_TypeDef *SPIX)` function instead.
  *
  * @param None
  * @retval None
  */
#ifdef HAL_SPI_MODULE_ENABLED
static void SPI_Init(void)
{
  /* SPI1 parameter configuration*/
	spiadc.hspiX.Instance = spiadc.SPIinstance;
	spiadc.hspiX.Init.Mode = SPI_MODE_MASTER;
	spiadc.hspiX.Init.Direction = SPI_DIRECTION_2LINES;
	spiadc.hspiX.Init.DataSize = SPI_DATASIZE_8BIT;
	spiadc.hspiX.Init.CLKPolarity = SPI_POLARITY_LOW;
	spiadc.hspiX.Init.CLKPhase = SPI_PHASE_1EDGE;
	spiadc.hspiX.Init.NSS = SPI_NSS_SOFT;
	spiadc.hspiX.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
	spiadc.hspiX.Init.FirstBit = SPI_FIRSTBIT_MSB;
	spiadc.hspiX.Init.TIMode = SPI_TIMODE_DISABLE;
	spiadc.hspiX.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	spiadc.hspiX.Init.CRCPolynomial = 10;
  HAL_SPI_Init(&spiadc.hspiX);
}
#endif

/**
  * @brief Это внутренняя функция библиотеки. Она включает выбранный канал АЦП.
  * 				Из программы к ней обращаться не нужно. Используйте GetADC(uint32_t Channel) /
  * 				This is an internal function of the library. It turns on the selected ADC channel.
  * 				There is no need to access it from the program. Use GetADC(uint32_t Channel)
  *
  * @param uint32_t Channel – номер канала АЦП: ADC_CHANNEL_0, ADC_CHANNEL_1,…  /
  *        uint32_t Channel – ADC channel number: ADC_CHANNEL_0, ADC_CHANNEL_1,…
  * @retval None
  */
#ifdef		HAL_ADC_MODULE_ENABLED
static void ADC_CHANNEL(uint32_t Channel)
{
	ADC_ChannelConfTypeDef sConfig = {0};
#ifdef __STM32F4xx_HAL_H
	sConfig.Channel = Channel;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;
#elif defined(__STM32F1xx_HAL_H)
	sConfig.Channel = Channel;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
#else
#error "There are no ADC channel configuration settings in void ADC_CHANNEL(uint32_t Channel)"
#endif
	HAL_ADC_ConfigChannel(&spiadc.hadcX, &sConfig);
}

/**
  * @brief SetADC – функция для инициализации АЦП / a function for initializing the ADC
  *
  * @param ADC_HandleTypeDef *hadc - указатель на структуру дескриптора АЦП /
  * 																 pointer to the ADC descriptor structure
  *
  * 	@param ADC_TypeDef  *ADCinstance - выбранный модуль АЦП: ADC1, ADC2,… /
  * 																	  selected ADC module: ADC1, ADC2,…
  * @retval None
  * @note
  */
void SetADC(ADC_HandleTypeDef *hadc, ADC_TypeDef  *ADCinstance)
{
	spiadc.hadcX = *hadc;
	spiadc.ADCinstance = ADCinstance;
	ADC_Init();
}
#endif

/**
  * @brief SetSPI - функция для инициализации SPI / a function for initializing the SPI
  *
  * @param (SPI_HandleTypeDef *HSPI - указатель на структуру дескриптора SPI /
  * 																  pointer to the SPI descriptor structure
  *
  * 	@param SPI_TypeDef *SPIX - выбранный модуль SPI: SPI1, SPI2,… /
  * 														selected SPI module: SPI1, SPI2,…
  * @retval None
  * @note
  */
#ifdef HAL_SPI_MODULE_ENABLED
void SetSPI(SPI_HandleTypeDef *HSPI, SPI_TypeDef *SPIX)
{
	spiadc.hspiX = *HSPI;
	spiadc.SPIinstance = SPIX;
	SPI_Init();
}
#endif

/**
  * @brief SetThermocouple – Настройка измерения температуры с помощью термопары и АЦП микроконтроллера /
  * 													Setup of temperature measurement using a thermocouple and a microcontroller ADC.
  *
  * @param ThermocoupleStruct * therm - Указатель на структуру, в которой хранятся настройки,
  * 				которые необходимы для использования термопары. /
  * 				pointer to a structure containing the settings required to use the thermocouple.
  * @note эту структуру необходимо создать в main.c /
  * 				this structure needs to be created in main.c.
  *
  * @param float Gain - Коэффициент усиления операционного усилителя, к которому подключена термопара /
  * 										 Gain of the operational amplifier to which the thermocouple is connected
  *
  * @param uint16_t ColdJunc - Температура холодного спая / Cold junction temperature.
  * @note 	Если вы измеряете реальную температуру холодного спая с помощью NTC или PTC
  * 				термистора – этот параметр должен равняться 0, или можно использовать макрос RCJ_ON.
  * 				Если вы хотите использовать термопару без измерения реальной температуры холодного
  * 				спая – укажите комнатную температуру.
  * 				/
  * 				If you are measuring the actual cold-junction temperature using an NTC or PTC thermistor,
  * 				this parameter should be set to 0, or the RCJ_ON macro can be used. If you wish to use
  * 				a thermocouple without measuring the actual cold-junction temperature,
  * 				specify the room temperature.
  *
  * @param uint16_t ZADC - Значение АЦП на выходе операционного усилителя при равной температуре
  * 				холодного и горячего спаев. /
  * 				ADC value at the operational amplifier output when the cold and hot junctions
  * 				are at the same temperature.
  * @param uint32_t channel1 - Канал АЦП, к которому подключена термопара: ADC_CHANNEL_0, ADC_CHANNEL_1,… /
  * 												ADC channel to which the thermocouple is connected: ADC_CHANNEL_0, ADC_CHANNEL_1,…
  *
  * @param const uint16_t *table - Указатель на таблицу значений термо-ЭДС в зависимости от температуры
  * 															 Pointer to a table of thermoelectric EMF values ​​versus temperature
  * @note Температура в таблице должна быть указана в микровольтах. Если необходимо измерять ткмпературу
  * 				ниже 0 градусов по Цельсию – в функции Get_temperature(ThermocoupleStruct * therm, THERM_TYPE type)
  * 				необходимо отредактировать условный оператор: if(therm->TERMTABLE[i] >= T1) { T1 = i; break; }.
  * 				Если минимальное значение температуры в таблице ниже 0, например -100 – строку «T1 = i;»
  * 				необходимо заменить на «T1 = i - 100;»
  * 				/
  * 				The temperature values ​​in the table must be specified in microvolts. If it is necessary to measure
  * 				temperatures below 0 degrees Celsius, the conditional statement within
  * 				the `Get_temperature(ThermocoupleStruct * therm, THERM_TYPE type)` function must be modified:
  * 				«if(therm->TERMTABLE[i] >= T1) { T1 = i; break; }». If the minimum temperature value in
  * 				the table is below 0 (e.g., -100), the line «T1 = i;» must be replaced with «T1 = i - 100;».
  *
  * @param uint16_t SizeTable - Размер таблицы (общее количество значений термо-ЭДС) /
  * 														Table size (total number of thermoelectric EMF values)
  * @retval None
  */
#ifdef		HAL_ADC_MODULE_ENABLED
void SetThermocouple (ThermocoupleStruct * therm, float Gain, uint16_t ColdJunc, uint16_t ZADC,\
																	uint32_t channel1, const uint16_t *table, uint16_t SizeTable)
{
	therm->ADCchannel = channel1;
	therm->ZeroADC = ZADC;
	therm->gain = Gain;
	therm->TERMTABLE = table;
	therm->SizeTable = SizeTable;
	if(ColdJunc) {
		therm->TempColdJunc = ColdJunc;
		therm->RCJ = RCJ_OFF;
	} else {
		therm->RCJ = RCJ_ON;
	}
}

/**
  * @brief SetThermistor - Настройка измерения температуры при помощи термистора /
  * 												Configuring temperature measurement using a thermistor
  *
  * @param ThermocoupleStruct * therm - Указатель на структуру, в которой хранятся настройки,
  * 				которые необходимы для использования термопары. /
  * 				pointer to a structure containing the settings required to use the thermocouple.
  *
  * @param 	const uint16_t *R1table - Указатель на таблицу сопротивлений в зависимости от температуры /
  * 																		Reference to the temperature-resistance table
  *
  * @param uint16_t SizeTable - Размер таблицы (общее количество значений сопротивления) /
  *  													Table size (total number of resistance values)
  *
  * @param uint32_t r2 - сопротивление резистора R2 / resistance of resistor R2
  * @note Для измерения температуры при помощи термистора необходимо NTC, или PTC термистор
  * 				подключить одним контактом к VDD микроконтроллера, а вторым к резистору R2 и входу
  * 				АЦП микроконтроллера. Второй контакт резистора R2 подключается к GND. Таким образом
  * 				выходит делитель напряжения, верхнее плечо которого – это термистор, а
  * 				нижнее – резистор R2. ВАЖНО! Сопротивление резистора R2 необходимо указывать в тех
  * 				единицах, в которых указаны значения сопротивления в зависимости от температуры в
  * 				таблице. Например, если в массиве PT100x100[351] сопротивление указано в Ом*100,
  * 				а резистор R2 у вас 1 кОм - uint32_t r2 должно равняться 100000. /
  * 				To measure temperature using an NTC or PTC thermistor, connect one terminal of
  * 				the thermistor to the microcontroller's VDD pin and the other to resistor R2 and
  * 				the microcontroller's ADC input. Connect the other terminal of resistor R2 to GND.
  * 				This configuration creates a voltage divider where the thermistor forms the upper
  * 				leg and resistor R2 forms the lower leg. IMPORTANT! The resistance value specified
  * 				for R2 must use the same units as the resistance values ​​listed in the temperature
  * 				lookup table. For example, if the `PT100x100[351]` array stores resistance values ​​
  * 				in Ohms × 100 and you are using a 1 kOm resistor for R2, then `uint32_t r2` should
  * 				be set to 100000.
  *
  * @param uint8_t ntcptc - Тип термистора: NTC, или PTC / Thermistor type: NTC, or PTC
  *
  * @param uint32_t channel1 - Канал АЦП, к которому подключен термистора: ADC_CHANNEL_0, ADC_CHANNEL_1,…
  * 													ADC channel to which the thermistor is connected: ADC_CHANNEL_0, ADC_CHANNEL_1,…
  *
  * @retval None
  */
void SetThermistor (ThermocoupleStruct * therm, 	const uint16_t *R1table, uint16_t SizeTable,\
																							uint32_t r2, uint8_t ntcptc, uint32_t channel1)
{
	therm->ADCchannel = channel1;
	therm->TERMTABLE = R1table;
	therm->SizeTable = SizeTable;
	therm->thermistor = ntcptc;
	therm->__R2 = r2;
	ADC_CHANNEL(therm->ADCchannel);
}
#endif

/**
  * @brief SetMAX6675 - настройка измерения температуры с помощью микросхемы MAX6675 /
  * 										Configuring temperature measurement using the MAX6675 chip
  *
  * @param ThermocoupleStruct * therm - Указатель на структуру, в которой хранятся настройки,
  * 				которые необходимы для использования термопары. /
  * 				pointer to a structure containing the settings required to use the thermocouple.
  *
  * @param GPIO_TypeDef *gpioCS - порт GPIO, к которому подключен CS микросхемы MAX6675: GPIOA, GPIOB,... /
  * 															GPIO port to which the MAX6675 chip's CS pin is connected: GPIOA, GPIOB,...
  *
  * @param uint16_t CS_pin - номер вывода порта GPIO, к которому подключен CS микросхемы MAX6675:GPIO_PIN_0,
  * 													GPIO_PIN_2,...GPIO_PIN_15 /
  * 													GPIO port pin number to which the MAX6675 chip's CS pin is connected:
  * 													GPIO_PIN_0, GPIO_PIN_2, ... GPIO_PIN_15
  *
  * @retval None
  */
#ifdef HAL_SPI_MODULE_ENABLED
void SetMAX6675(ThermocoupleStruct * therm, GPIO_TypeDef *gpioCS, uint16_t CS_pin)
{
	therm->GPIOSPICS = gpioCS;
	therm->PINSPICS = CS_pin;
}
#endif


/**
  * @brief SetTermColdJunc - Настройка измерения температуры холодного спая /
  * 													Setting up cold junction temperature measurement
  *
  * @param ThermocoupleStruct * therm - Указатель на структуру, в которой хранятся настройки,
  * 				которые необходимы для использования термопары. /
  * 				pointer to a structure containing the settings required to use the thermocouple.
  *
  * @param uint32_t channel2 - Канал АЦП, к которому подключен термистор измеряющий температуру
  * 														холодного спая: ADC_CHANNEL_0, ADC_CHANNEL_1,…
  * 														The ADC channel to which the thermistor measuring the cold-junction
  * 														temperature is connected: ADC_CHANNEL_0, ADC_CHANNEL_1,…
  *
  * @param 	const uint16_t *R1table - Указатель на таблицу сопротивлений в зависимости от температуры /
  * 																		Reference to the temperature-resistance table
  *
  * @param uint16_t SizeTable - Размер таблицы (общее количество значений сопротивления) /
  *  													Table size (total number of resistance values)
  *
  * @param uint32_t r2 - сопротивление резистора R2 / resistance of resistor R2
  * @note Для измерения температуры холодного спая необходимо NTC, или PTC термистор
  * 				подключить одним контактом к VDD микроконтроллера, а вторым к резистору R2 и входу
  * 				АЦП микроконтроллера. Второй контакт резистора R2 подключается к GND. Таким образом
  * 				выходит делитель напряжения, верхнее плечо которого – это термистор, а
  * 				нижнее – резистор R2. ВАЖНО! Сопротивление резистора R2 необходимо указывать в тех
  * 				единицах, в которых указаны значения сопротивления в зависимости от температуры в
  * 				таблице. Например, если в массиве PT100x100[351] сопротивление указано в Ом*100,
  * 				а резистор R2 у вас 1 кОм - uint32_t r2 должно равняться 100000. /
  * 				To measure temperature cold junction, connect one terminal of
  * 				the thermistor to the microcontroller's VDD pin and the other to resistor R2 and
  * 				the microcontroller's ADC input. Connect the other terminal of resistor R2 to GND.
  * 				This configuration creates a voltage divider where the thermistor forms the upper
  * 				leg and resistor R2 forms the lower leg. IMPORTANT! The resistance value specified
  * 				for R2 must use the same units as the resistance values ​​listed in the temperature
  * 				lookup table. For example, if the `PT100x100[351]` array stores resistance values ​​
  * 				in Ohms × 100 and you are using a 1 kOhm resistor for R2, then `uint32_t r2` should
  * 				be set to 100000.
  *
  * @param uint8_t ntcptc - Тип термистора: NTC, или PTC / Thermistor type: NTC, or PTC
  *
  * @retval None
  */
#ifdef		HAL_ADC_MODULE_ENABLED
void SetTermColdJunc (ThermocoupleStruct * therm, uint32_t channel2, const uint16_t *R1table, \
																							uint16_t SizeTable, uint32_t r2, uint8_t ntcptc)
{
	therm->ADC_Zero_channel = channel2;
	therm->R1TABLE = R1table;
	therm->R2 = r2;
	therm->SizeR1table = SizeTable;
	therm->thermistorCJ = ntcptc;
}
#endif

/**
  * @brief Get_temperature - функция измеряет температуру и возвращает ее значение в градусах Цельсия. /
  * 													The function measures the temperature and returns its value in degrees Celsius.
  *
  * @param ThermocoupleStruct * therm - Указатель на структуру, в которой хранятся настройки,
  * 				которые необходимы для использования термопары. /
  * 				pointer to a structure containing the settings required to use the thermocouple.
  *
  * @param THERM_TYPE type - тип термодатчика, которым измеряем температуру:
  * 													THREM_K - термопара типа К. Прямое измерение с
  * 													помощью АЦП микроконтроллера;
  * 													MAX6675 - термопара типа К. Измерение с использованием
  * 													микросхемы MAX6675
  * 													THERMISTOR - NTC или PTC термистор. /
  * 													Type of temperature sensor used for measurement:
	*													THREM_K – Type K thermocouple. Direct measurement
	*													using the microcontroller's ADC;
	*													MAX6675 – Type K thermocouple. Measurement using the MAX6675 chip;
	*													THERMISTOR – NTC or PTC thermistor.
  *
  * @retval Температура в градусах Цельсия /
  * 					Temperature in degrees Celsius
  *
  */
uint16_t Get_temperature(ThermocoupleStruct * therm, THERM_TYPE type)
{
#ifdef		HAL_ADC_MODULE_ENABLED
	if(type == THREM_K) {
		float T1 = GetADC(therm->ADCchannel);
		if(!therm->RCJ) {
			therm->TempColdJunc = GetTempColdJunc(therm);
		}
		T1 -= therm->ZeroADC;
		T1 = T1 * Vmax * 1000000 / 4095 / therm->gain;
		T1 += therm->TERMTABLE[therm->TempColdJunc];
		for (uint16_t i = 0; i < therm->SizeTable; i++) {
			if(therm->TERMTABLE[i] >= T1) {
				T1 = i;
				break;
			}
		}
		return (uint16_t)T1;
#endif
#ifdef HAL_SPI_MODULE_ENABLED
#ifdef		HAL_ADC_MODULE_ENABLED
	} else if (type == MAX6675) {
#endif
		uint16_t v;
		uint8_t tmp[2];
		HAL_GPIO_WritePin(therm->GPIOSPICS, therm->PINSPICS, GPIO_PIN_RESET);	// set cs to low
		HAL_SPI_Receive(&spiadc.hspiX, &tmp[0], 2, 10);		// get data from MAX6675
		HAL_GPIO_WritePin(therm->GPIOSPICS, therm->PINSPICS, GPIO_PIN_SET);	// set cs to high
		v = tmp[0];
		v <<= 8;
		v |= tmp[1];
		if (v & 0x4) {
				return 0xFFFF;	// Thermocouple not connected!
		} else {
				v >>= 3;
				v /= 4;
				return v;
		}
#endif
#ifdef		HAL_ADC_MODULE_ENABLED
	} else if (type == THERMISTOR) {
		float T1 = GetADC(therm->ADCchannel);
		T1 = 4095 * therm->__R2 / T1 - therm->__R2;
		if(therm->thermistor == NTC) {
			for(uint16_t i = 0; i < therm->SizeTable; i++) {
				if(therm->TERMTABLE[i] <= T1) {
					T1 = i;
					break;
				}
			}
		} else if (therm->thermistor == PTC) {
			for(uint16_t i = 0; i < therm->SizeTable; i++) {
				if(therm->TERMTABLE[i] >= T1) {
					T1 = i;
					break;
				}
			}
		}
		return (uint8_t) T1;
	}else {
		return 0;
	}
#endif
}

/**
  * @brief GetTempColdJunc - функция измеряет температуру холодного спая и возвращает
  * 													значение в градусах Цельсия /
  * 													The function measures the cold-junction temperature and
  * 													returns the value in degrees Celsius.
  *
  * @param ThermocoupleStruct * therm - Указатель на структуру, в которой хранятся настройки,
  * 				которые необходимы для использования термопары. /
  * 				pointer to a structure containing the settings required to use the thermocouple.
  *
  * @retval Температура в градусах Цельсия /
  * 					Temperature in degrees Celsius
  *
  */
/* R1=Vin*R2/Vout-R2 */
#ifdef		HAL_ADC_MODULE_ENABLED
uint8_t GetTempColdJunc (ThermocoupleStruct * therm)
{
	float T1 = GetADC(therm->ADC_Zero_channel);
	T1 = 4095 * therm->R2 / T1 - therm->R2;
	if(therm->thermistorCJ == NTC) {
		for(uint16_t i = 0; i < therm->SizeR1table; i++) {
			if(therm->R1TABLE[i] <= T1) {
				T1 = i;
				break;
			}
		}
	} else if (therm->thermistorCJ == PTC) {
		for(uint16_t i = 0; i < therm->SizeR1table; i++) {
			if(therm->R1TABLE[i] >= T1) {
				T1 = i;
				break;
			}
		}
	}
	return (uint8_t) T1;
}

/**
  * @brief GetADC - функция измеряет напряжение на указанном входе АЦП /
  * 								The function measures the voltage at the specified ADC input.
  *
  * @param uint32_t Channel - Канал АЦП: ADC_CHANNEL_0, ADC_CHANNEL_1,… /
  * 													ADC channel: ADC_CHANNEL_0, ADC_CHANNEL_1,…
  *
  * @retval Значение АЦП (0 - 4095)
  * 				  ADC value (0 - 4095)
  * @note данную функцию удобно использовать для компенсации смещения нуля на
  * 				выходе операционного усилителя. Для этого достаточно считать значение АЦП
  * 				на входе, к которому подключена термопара, пока она холодная и температуры
  * 				горячего и холодного спаев равны /
  * 				This function is convenient for compensating for the zero-offset at the
  * 				operational amplifier output. To do this, simply read the ADC value at
  * 				the input to which the thermocouple is connected while it is cold and
  * 				the temperatures of the hot and cold junctions are equal.
  *
  */
uint16_t GetADC(uint32_t Channel)
{
	uint16_t adc;
	ADC_CHANNEL(Channel);
	HAL_ADC_Start(&spiadc.hadcX);
	HAL_ADC_PollForConversion(&spiadc.hadcX, 100);
	adc = HAL_ADC_GetValue(&spiadc.hadcX);
	HAL_ADC_Stop(&spiadc.hadcX);
  return adc;
}

/**
  * @brief SetVRef - настройка внешнего источника опорного напряжения /
  * 									configuration of an external reference voltage source
  * @note Внешний источник опорного напряжения подключается к любому входу АЦП
  * 				An external reference voltage source is connected to any ADC input.
  *
  * @param float Uref - эталонное напряжение, которое выдаёт внешний источник
  * 										опорного напряжения. Для TL431 это 2.495В
  * 										the reference voltage provided by an external reference
  * 										voltage source. For the TL431, this is 2.495 V.
  *
  * 	@param uint32_t Channel - Канал АЦП: ADC_CHANNEL_0, ADC_CHANNEL_1,… /
  * 													ADC channel: ADC_CHANNEL_0, ADC_CHANNEL_1,…
  *
  * @retval None
  *
  */
void SetVRef(float Uref, uint32_t Channel)
{
	float adc = 0;
	for(uint8_t i = 0; i < 5; i++) {
		adc += GetADC(Channel);
		HAL_Delay(100);
	}
	adc /= 5;
	Vmax = Uref * 4095 / adc;
}
#endif
