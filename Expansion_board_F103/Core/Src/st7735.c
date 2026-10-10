/**
  ******************************************************************************
  * @file    ST7735.cpp
  * @author  Θεοδοτος (Theodotos) или просто Богдан Александрович
  * @version V1.0
  * @date    16-мая-2024
  * @brief   Библиотека для управления TFT дисплеем 1.44" с разрешением 128*128p,
  *          на драйвере ST7735.
  *
  *          ------------------------------------------------------------------
  *
  *          Данная библиотека подходит для программирования под микроконтроллер
  *          в AtmelStudio 7.0 (в этой IDE написана) на языке C++, а также для
  *          программирования под Arduino в среде разработки Arduino IDE,
  *			 проверена в Arduino IDE 2.3.2 с платой Arduino nano
  *          (китайская копия). В AtmelStudio 7.0 проверена с этой-же платой, а
  *          значит работает с микроконтроллером Atmega328P с кварцем на 16 МГц
  *
  *          ------------------------------------------------------------------
  *          Для работы в Arduino IDE в файле ST7735.h должна быть
  *          раскомментирована строка: #include "Arduino.h", соответственно,
  *          для работы в AtmelStudio 7.0 она должна быть закомментирована.
  *
  *          ------------------------------------------------------------------
  *          Для работы в AtmelStudio 7.0 с микроконнтроллером несовместимым по
  *          выводам с Atmega328P необходимо переопределить макросы порта
  *          управления дисплеем для корректной работы аппаратного SPI - в
  *          файле ST7735.h настраиваем под свой контроллер макросы:
  *          #define LCD_PORT		PORTB
  *          #define LCD_DDR		DDRB
  *          Кроме этого необходимом переопределить частоту под свой кварц для
  *          библиотуки delay.h, файл ST7735.cpp, макрос: #define F_CPU 16000000UL
  *          Также в конструкторе класса ST7735, в этом файле (ST7735.cpp)стоит
  *          проверка выбранных выводов на соответствие аппаратному SPI.
  *          Для Arduino IDE это строка:
  *          if ((LCD_SDA == 11) && (LCD_SCK == 13)) {
  *          Для AtmelStudio:
  *          if ((LCD_SDA == 3) && (LCD_SCK == 5)) {
  *          // PORTB3 - это MOSI, а PORTB5 - SCK аппаратного SPI Atmega328P,
  *          на платах Arduino: D11 - это MOSI, а D13 - SCK. При выборе любых
  *          других выводов библиотека автоматически перейдёт к использованию
  *          програмного SPI, с кварцем 16MHz он корректно не работает с этим
  *          дисплеем, при этом аппаратный работает отлично.
  *          Также для использования библиотеки с микроконтроллером отличным
  *          от Atmega328P необходимо проверить на соответствие даташиту
  *          функцию инициализации аппаратного SPI:
  *          void ST7735::InitSPI ()
  *          и возможно функцию передачи данных:
  *          void ST7735::spiTransmite (unsigned char data)
  *          остальные функции не аппаратно зависимы и должны корректно работать
  *          с любым контроллером Atmega.
  ******************************************************************************
  **/


#include "stdio.h"
#include "string.h"
#include "ST7735.h"
#include "Font_8x16.h"


/* Глобальные переьенные -----------------------------------------------------*/
  volatile unsigned int MAX_X;
  volatile unsigned int MAX_Y;
  volatile unsigned int LCD_SIZE;
  volatile unsigned int _RGB;

  volatile unsigned int Tmp_x = 0;
  volatile unsigned int Tmp_y = 0;

  bool transporent = TRANSPORENT_OFF;

  uint8_t SPI_Type, ST_SDA, ST_SCK, ST_CS, ST_RES, ST_RS;
  uint8_t LCD_screen_row_start = 0;
  uint8_t LCD_screen_column_start = 0;
  uint8_t LCD_row_start = 0;
  uint8_t LCD_column_start = 0;
  uint16_t LCD_width = 0;
  uint16_t LCD_height = 0;
  uint8_t Xoffs = 2;
  uint8_t Yoffs = 3;
  uint8_t XinvOffs = 2;
  uint8_t YinvOffs = 1;
  uint8_t orientation = PORTRAIT;
  char Temp0;
  DisplaySettings dispSet;
  ST7735struct st7735;

/*******************************************************************************
* Название функции   : Конструктор ST7735
* Описание функции   : Инициализирует порты и основные настройки по умолчанию
*                    : для работы библиотеки
* Функция принимает  : LCD_SDA - намер вывода МК для кнтакта дисплея SDA
*                    : LCD_SCK - намер вывода МК для кнтакта дисплея SCK
*                    : LCD_CS - намер вывода МК для кнтакта дисплея CS
*                    : LCD_RES - намер вывода МК для кнтакта дисплея RES
*                    : LCD_RS - намер вывода МК для кнтакта дисплея RS
* Функция возвращает : пусто
*******************************************************************************/


void LCD(SPI_HandleTypeDef *hspi, GPIO_TypeDef *port, uint16_t pin_cs, uint16_t pin_rs, uint16_t pin_rst)
{
  st7735.hspi = hspi;
  st7735.port = port;
  st7735.pin_cs = pin_cs;
  st7735.pin_rs = pin_rs;
  st7735.pin_rst = pin_rst;

  // Настройки по умолчанию, в дальнейшем можно будет изменить
  dispSet.color = WHITE;
  dispSet.bColor = BLACK;
  dispSet.font = font_8x16;
  dispSet.x_size = dispSet.font[0];
  dispSet.y_size = dispSet.font[1];
  dispSet.offset = dispSet.font[2];
  dispSet.numchars = dispSet.font[3];
}


/*******************************************************************************
* Название функции   : spiTransmite (private)
* Описание функции   : Передаёт байт по шине SPI
* Функция принимает  : unsigned char data - 8-битная переменная дла отправки по
*                    : шине SPI
* Функция возвращает : пусто
*******************************************************************************/
void spiTransmite (unsigned char data) {
  HAL_SPI_Transmit(st7735.hspi, &data, 1, 10);
}

/*******************************************************************************
* Название функции   : LCD_CMD(private)
* Описание функции   : Передаёт команду дисплею
* Функция принимает  : unsigned char cmd - 8-битная команда
* Функция возвращает : пусто
*******************************************************************************/
void LCD_CMD (unsigned char cmd)
{
#ifdef TransferWithoutChecks
	st7735.hspi->Instance->CR1 &= ~(SPI_CR1_BR_Msk);
#endif
  WRITE_CMD;			// RS(A0)=0
  SELECT_LCD;			// CS=0
  spiTransmite(cmd);
  DESELECT_LCD;		// CS=1
#ifdef TransferWithoutChecks
  st7735.hspi->Instance->CR1 |= (SPI_BAUDRATEPRESCALER_32 & SPI_CR1_BR_Msk);
#endif
}



/*******************************************************************************
* Название функции   : LCD_DATA(private)
* Описание функции   : Передаёт команду дисплею
* Функция принимает  : unsigned char dat - 8-битная переменная дла отправки
*                    : данных дисплею
* Функция возвращает : пусто
*******************************************************************************/
void LCD_DATA (unsigned char dat)
{
#ifdef TransferWithoutChecks
	st7735.hspi->Instance->CR1 &= ~(SPI_CR1_BR_Msk);
#endif
  WRITE_DATA;			// RS(A0)=1
  SELECT_LCD;			// CS=0
  spiTransmite(dat);
  DESELECT_LCD;		// CS=1
#ifdef TransferWithoutChecks
  st7735.hspi->Instance->CR1 |= (SPI_BAUDRATEPRESCALER_32 & SPI_CR1_BR_Msk);
#endif
}

/*******************************************************************************
* Название функции   : Write_Color(private)
* Описание функции   : Передаёт цвет одного пикселя дисплею
* Функция принимает  : uint16_t color - 16-битная переменная дла отправки
*                    : цвета одного пикселя дисплею
* Функция возвращает : пусто
*******************************************************************************/
void Write_Color(uint16_t color)
{
#ifdef TransferWithoutChecks
	st7735.hspi->Instance->CR1 &= ~(SPI_CR1_BR_Msk);
#endif
  spiTransmite(color >> 8);
  spiTransmite(color);
#ifdef TransferWithoutChecks
  st7735.hspi->Instance->CR1 |= (SPI_BAUDRATEPRESCALER_32 & SPI_CR1_BR_Msk);
#endif
}

/*******************************************************************************
* Название функции   : Init (public)
* Описание функции   : Функция вызывается для инициализации дислея
*                    : с указанием разрешения: 128х128 или 128х160
*                    : цвет передается в формате BGR
*                    : после объявления конструктора ST7735
* Функция принимает  : bool Size - true - дисплей с разрешением 128х128
*                    : false -  - дисплей с разрешением 128х160
*                    : для удобства можно передавать в функцию LCD128X128 или LCD128X160
*                    : unsigned char RGB_SET - формат передачи цвета дисплею,
*                    : могут быть значения RGB или BGR
* Функция возвращает : пусто
*******************************************************************************/
void Init (bool Size, unsigned char RGB_SET)
{
  SELECT_LCD;
  if(st7735.pin_rst)LCD_RES_HI;

  LCD_CMD(ST7735_SWRESET);	// 0x01
  _delay_ms(130);
  LCD_CMD(ST7735_SLPOUT);		// 0x11
  _delay_ms(130);
  LCD_CMD(ST7735_FRMCTR1);	// 0xB1
  LCD_DATA(0x01);
  LCD_DATA(0x2C);
  LCD_DATA(0x2D);
  LCD_CMD(ST7735_FRMCTR2);	// 0xB2
  LCD_DATA(0x01);
  LCD_DATA(0x2C);
  LCD_DATA(0x2D);
  LCD_CMD(ST7735_FRMCTR3);	// 0xB3
  LCD_DATA(0x01);
  LCD_DATA(0x2C);
  LCD_DATA(0x2D);
  LCD_DATA(0x01);
  LCD_DATA(0x2C);
  LCD_DATA(0x2D);
  LCD_CMD(ST7735_INVCTR);		// 0xB4
  LCD_DATA(0x07);
  LCD_CMD(ST7735_PWCTR1);		// 0xC0
  LCD_DATA(0xA2);
  LCD_DATA(0x02);
  LCD_DATA(0x84);
  LCD_CMD(ST7735_PWCTR2);		// 0xC1
  LCD_DATA(0xC5);
  LCD_CMD(ST7735_PWCTR3);		// 0xC2
  LCD_DATA(0x0A);
  LCD_DATA(0x00);
  LCD_CMD(ST7735_PWCTR4);		// 0xC3
  LCD_DATA(0x8A);
  LCD_DATA(0x2A);
  LCD_CMD(ST7735_PWCTR5);		// 0xC4
  LCD_DATA(0x8A);
  LCD_DATA(0xEE);
  LCD_CMD(ST7735_VMCTR1);		// 0xC5
  LCD_DATA(0x0E);
  LCD_CMD(ST7735_INVOFF);		// 0x20
  LCD_CMD(ST7735_MADCTL);		// 0x36
  LCD_DATA(0xC8);
  LCD_CMD(ST7735_COLMOD);		// 0x3A
  LCD_DATA(0x05);

  //st7735_run_command_list(st7735_red_init_green1442);
  LCD_CMD(ST7735_CASET);		// 0x2A
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  if(Size) {
    LCD_DATA(0x7F);
  } else {
    LCD_DATA(0x9F);
  LCD_CMD(ST7735_RASET);		// 0x2B
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x7F);
  }

  //st7735_run_command_list(st7735_red_init3);
  LCD_CMD(ST7735_GMCTRP1);	// 0xE0
  LCD_DATA(0x02);
  LCD_DATA(0x1C);
  LCD_DATA(0x07);
  LCD_DATA(0x12);
  LCD_DATA(0x37);
  LCD_DATA(0x32);
  LCD_DATA(0x29);
  LCD_DATA(0x2D);
  LCD_DATA(0x29);
  LCD_DATA(0x25);
  LCD_DATA(0x2B);
  LCD_DATA(0x39);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_CMD(ST7735_GMCTRP1);	// 0xE0
  LCD_DATA(0x03);
  LCD_DATA(0x1D);
  LCD_DATA(0x07);
  LCD_DATA(0x06);
  LCD_DATA(0x2E);
  LCD_DATA(0x2C);
  LCD_DATA(0x29);
  LCD_DATA(0x2D);
  LCD_DATA(0x2E);
  LCD_DATA(0x2E);
  LCD_DATA(0x37);
  LCD_DATA(0x3F);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x02);
  LCD_DATA(0x10);
  LCD_CMD(ST7735_NORON);		// 0x13
  _delay_ms(10);
  LCD_CMD(ST7735_DISPON);		// 0x29
  _delay_ms(130);

  LCD_width = MAX_Y = 127;
  if(Size) {
    LCD_SIZE = 1;
    LCD_height = MAX_X = 127;
  } else {
    LCD_SIZE = 0;
    LCD_height = MAX_X = 159;
  }
  _RGB = RGB_SET;
  SetOrientation(PORTRAIT);
}

/*******************************************************************************
* Название функции   : SetOffset (public)
* Описание функции   : Функция устанавливает смещение для осей X и Y, так как
*                    : изначально оно может быть некорректным. При чем отдельно
*                    : устанавливается смещение для портретной и пейзажной
*                    : ориентации, и отдельно для инвертированной портретной и
*                    : инвертированной пейзажной. Пользователь передаёт только
*                    : значение в пикселях, для смещения вправо и вниз -
*                    : положительные значения, влево и вверх - отрицательные.
*                    : Функция сама определяет, куда записать переданные значения
*                    : в зависимости от выбранной ориентации в данный момент.
*                    : изменения вступают в силу при следующем обращении к
*                    : любой функции, которая выводит что-то на экран.
*					 : По умолчанию установлены следующие значения смещения:
*                    : Xoffs = 2; Yoffs = 3; XinvOffs = 2; YinvOffs = 1;
*                    : Если в вашем случае смещение не корректно - лучше всего,
*                    : после подбора нужных значений, обратиться к функции
*                    : SetOffset() один раз перед инициализацией дисплея, или
*                    : по необходимости два раза, вначале установив смещение
*                    : для нормального режима, потом вызвать функцию
*                    : SetOrientation(PORTRAIT_INV), таким образом установить
*                    : инвертированную ориентацию и установить смещение для
*                    : неё. После этого вызвать функцию инициализации дисплея
*                    : В таком случае смещение будет корректным во всех режимах
*                    : и на протяжении всей программы его настраивать больше не
*                    : понадобится.
* Функция принимает  : signed char offsX - смещение для оси X
*                    : signed char offsY - смещение для оси Y
* Функция возвращает : пусто
*******************************************************************************/
void SetOffset (signed char offsX, signed char offsY)
{
  if ((orientation == PORTRAIT) || (orientation == LANDSCAPE))
  {
    Xoffs = offsX;
    Yoffs = offsY;
  }
  else
  {
    XinvOffs = offsX;
    YinvOffs = offsY;
  }
}

/*******************************************************************************
* Название функции   : SetOrientation (public)
* Описание функции   : Функция включает выбранную ориентацию дисплея. При
*                    : инициализации устанавливается ориентация по умолчанию:
*                    : PORTRAIT, но её в любой момент можно изменить из программы,
*                    : обратившись к этой функции.
* Функция принимает  : unsigned char orient - ориентация дисплея. Макросы для
*                    : данной переменной:
*                    : PORTRAIT
*                    : LANDSCAPE
*                    : PORTRAIT_INV
*                    : LANDSCAPE_INV
* Функция возвращает : пусто
*******************************************************************************/
void SetOrientation(unsigned char orient)
{
  orientation = orient;
  LCD_CMD(ST7735_MADCTL);
  switch(orient) {
    case PORTRAIT:
    LCD_width = MAX_X = 127;
    if(LCD_SIZE) {
      LCD_height = MAX_Y = 127;
    } else {
      LCD_height = MAX_Y = 159;
    }
    LCD_DATA(MADCTL_MX | MADCTL_MY | _RGB);		// 0xC8
    LCD_column_start = LCD_screen_column_start = Xoffs;
    LCD_row_start = LCD_screen_row_start = Yoffs;
    break;
    case LANDSCAPE:
    LCD_height = MAX_Y = 127;
    if(LCD_SIZE) {
      LCD_width = MAX_X = 127;
    } else {
      LCD_width = MAX_X = 159;
    }
    LCD_DATA(MADCTL_MY | MADCTL_MV | _RGB);		// 0xA8
    LCD_column_start = LCD_screen_row_start = Yoffs;
    LCD_row_start = LCD_screen_column_start = Xoffs;
    break;
    case PORTRAIT_INV:
    LCD_width = MAX_X = 127;
    if(LCD_SIZE) {
      LCD_height = MAX_Y = 127;
    } else {
      LCD_height = MAX_Y = 159;
    }
    LCD_DATA(_RGB);								// 0x08
    LCD_column_start = LCD_screen_column_start = XinvOffs;
    LCD_row_start = LCD_screen_row_start = YinvOffs;
    break;
    case LANDSCAPE_INV:
    LCD_height = MAX_Y = 127;
    if(LCD_SIZE) {
      LCD_width = MAX_X = 127;
    } else {
      LCD_width = MAX_X = 159;
    }
    LCD_DATA(MADCTL_MX | MADCTL_MV | _RGB);		// 0x68
    LCD_column_start = LCD_screen_row_start = YinvOffs;
    LCD_row_start = LCD_screen_column_start = XinvOffs;
    break;
  }
}


/*******************************************************************************
* Название функции   : SetColor (public)
* Описание функции   : Функция устанавливает цвет, который функция Print будет
*                    : использовать при выводе текста, если не задан другой.
* Функция принимает  : unsigned int color - 16-битная переменная задающая цвет
*                    : в формате 5-6-5 (rrrrrggggggbbbbb)
*                    : макросы для переменной color:
*                    : RED	        (0xF800)
*                    : GREEN        (0x07E0)
*                    : BLUE         (0x001f)
*                    : BLACK        (0x0000)
*                    : YELLOW       (0xffe0)
*                    : WHITE        (0xffff)
*                    : CYAN         (0x07ff)
*                    : BRIGHT_RED   (0xf810)
*                    : VIOLET       (0xC318)
*                    : GRAY         (0x8410)
*                    : GRAY1        (0x18C3)
*                    : BROWN        (0x59A4)
*                    : TURQUOISE    (0x2DFF)
*                    : MAGENTA      (0xF81F)
*                    : Также можно добавить свои макросы в файле ST7735.h или
*                    : задать любой цвет в формате 16-битной переменной
* Функция возвращает : пусто
*******************************************************************************/
void SetColor (unsigned int color)
{
  dispSet.color = color;
}

/*******************************************************************************
* Название функции   : SetBackColor (public)
* Описание функции   : Функция устанавливает цвет фона, который функция Print
*                    : будет использовать при выводе текста, если не задан
*                    : другой и не включена прозрачность (TRANSPORENT_ON).
*                    : Также это цвет будет использовать функция Clear() для
*                    : заливки дисплея.
* Функция принимает  : unsigned int color - 16-битная переменная задающая цвет
*                    : в формате 5-6-5 (rrrrrggggggbbbbb)
*                    : макросы для переменной color:
*                    : RED	        (0xF800)
*                    : GREEN        (0x07E0)
*                    : BLUE         (0x001f)
*                    : BLACK        (0x0000)
*                    : YELLOW       (0xffe0)
*                    : WHITE        (0xffff)
*                    : CYAN         (0x07ff)
*                    : BRIGHT_RED   (0xf810)
*                    : VIOLET       (0xC318)
*                    : GRAY         (0x8410)
*                    : GRAY1        (0x18C3)
*                    : BROWN        (0x59A4)
*                    : TURQUOISE    (0x2DFF)
*                    : MAGENTA      (0xF81F)
*                    : Также можно добавить свои макросы в файле ST7735.h или
*                    : задать любой цвет в формате 16-битной переменной
* Функция возвращает : пусто
*******************************************************************************/
void SetBackColor (unsigned int color)
{
  dispSet.bColor = color;
}

/*******************************************************************************
* Название функции   : SetFont (public)
* Описание функции   : Функция устанавливает шрифт для вывода текста на экран.
*                    : есть два шрифта: font_8x16 - символы 0-9,A-Z,a-z, А-Я,
*                    : а-я, а также основные знаки пунктуации; Размер символов
*                    : 8*16 пикселей
*                    : Также есть шрифт SevenSegNumFont - символы только 0-9,
*                    : Также с этим шрифтом можно поставить точку между
*                    : символами. Размер 24*41 пикселя.
* Функция принимает  : const unsigned char* font - указатель на массив данных
*                    : шрифта. Функции можно передать два значения:
*                    : font_8x16 или SevenSegNumFont
* Функция возвращает : пусто
*******************************************************************************/
void SetFont (const unsigned char* font)
{
  dispSet.font = font;
  dispSet.x_size = dispSet.font[0];
  dispSet.y_size = dispSet.font[1];
  dispSet.offset = dispSet.font[2];
  dispSet.numchars = dispSet.font[3];
}

/*******************************************************************************
* Название функции   : SetTransporent (public)
* Описание функции   : Функция включает или выключает прозрачность фона при
*                    : выводе текста. Если прозрачность отключена - при обращении
*                    : к функциям Print(...) фон каждого символа будет
*                    : закрашиваться цветом, который был установлен в качестве
*                    : цвета фона (BackColor). Если прозрачность включена -
*                    : символы будут выводиться поверх того изображения, которое
*                    : в данный момент на экране. Фон символов не будет
*                    : закрашиваться.
* Функция принимает  : bool Transporent - можно передавать true или макрос
*                    : TRANSPORENT_ON для включения прозрачности и false или
*                    : макрос TRANSPORENT_OFF - для отключения.
* Функция возвращает : пусто
*******************************************************************************/
void SetTransporent (bool Transporent)
{
  transporent = Transporent;
}

/*******************************************************************************
* Название функции   : SetDisplay (public)
* Описание функции   : Функция устанавливает: цвет текста,
*                    : цвет фона, шрифт и включает/отключает прозрачность.
* Функция принимает  : unsigned int color - цвет текста
*                    : unsigned int bColor - цвет фона
*                    : const unsigned char* font - указатель на шрифт
*                    : bool Transporent - TRANSPORENT_ON (true) или
*                    : TRANSPORENT_OFF (false)
* Функция возвращает : пусто
*******************************************************************************/
void SetDisplay (unsigned int color, unsigned int bColor, const unsigned char* font, bool Transporent)
{
  transporent = Transporent;
  dispSet.color = color;
  dispSet.bColor = bColor;
  SetFont(font);
}

/*******************************************************************************
* Название функции   : window (public)
* Описание функции   : Функция устанавливает границы окна для вывода изображения
*                    : или текста, а также устанавливает курсор в координаты
*                    : X=0 и Y=0.
* Функция принимает  : unsigned int XL - левая граница по оси X
*                    : unsigned int XR - правая граница по оси X
*                    : unsigned int YL - левая граница по оси Y
*                    : unsigned int YR - правая граница по оси Y
* Функция возвращает : пусто
*******************************************************************************/
void window(unsigned int XL, unsigned int XR, unsigned int YL, unsigned int YR)
{
  LCD_CMD(ST7735_CASET); // Column addr set
  LCD_DATA(0x00);
  LCD_DATA(XL + LCD_column_start);	// XSTART
  LCD_DATA(0x00);
  LCD_DATA(XR + LCD_column_start); // XEND

  LCD_CMD(ST7735_RASET); // Row addr set
  LCD_DATA(0x00);
  LCD_DATA(YL + LCD_row_start); // YSTART
  LCD_DATA(0x00);
  LCD_DATA(YR + LCD_row_start); // YEND

  LCD_CMD(ST7735_RAMWR); // write to RAM
}

/*******************************************************************************
* Название функции   : cursor (public)
* Описание функции   : Функция устанавливает курсор в соответствии с заданными
*                    : координатами, при этом правая и нижняя границы окна
*                    : устанавливаются максимальные, а левая и верхняя
*                    : в соответствии с координатами
* Функция принимает  : unsigned int x - координата по оси X
*                    : unsigned int y - координата по оси Y
* Функция возвращает : пусто
*******************************************************************************/
void cursor (unsigned int x, unsigned int y)
{
  Tmp_x=x;
  Tmp_y=y;
  window(x, MAX_X, y, MAX_Y);
}

/*******************************************************************************
* Название функции   : cursor_txt (public)
* Описание функции   : Функция устанавливает курсор в соответствии с заданными
*                    : координатами, при этом правая и нижняя границы окна
*                    : устанавливаются максимальные, а левая и верхняя
*                    : в соответствии с координатами, но в отличии от функции
*                    : cursor() - их значение принимается не в пикселях, а в
*                    : символах, в зависимости от выбранного шрифта. Таким
*                    : образом cursor_txt(0, 0) установит координаты X и Y
*                    : в верхний левый угол, а cursor_txt(1, 1) установит
*                    : значение координаты X=8, а Y=16, если выбран шрифт
*                    : font_8x16 или X=24, а Y=43 - если SevenSegNumFont
* Функция принимает  : unsigned int x - координата по оси X
*                    : unsigned int y - координата по оси Y
* Функция возвращает : пусто
*******************************************************************************/
void cursor_txt (unsigned int x, unsigned int y)
{
  x*=dispSet.x_size;
  y*=dispSet.y_size;
  Tmp_x=x;
  Tmp_y=y;
  window(x, MAX_X, y, MAX_Y);
}

/*******************************************************************************
* Название функции   : put_pixel (public)
* Описание функции   : Функция устанавливает курсор по заданным координатам и
*                    : закрашивает один пиксель. При этом координаты курсора
*                    : сохраняются в глобальных переменных Tmp_x и Tmp_y
* Функция принимает  : unsigned int X - координата по оси X
*                    : unsigned int X - координата по оси Y
*                    : unsigned int Color - цвет
* Функция возвращает : пусто
*******************************************************************************/
void put_pixel(unsigned int X, unsigned int Y, unsigned int Color)
{
  if(X < 0 || X >= LCD_width || Y < 0 || Y >= LCD_height){
    return;
  }
  cursor(X,Y);

  WRITE_DATA;
  SELECT_LCD;

  Write_Color(Color);

  DESELECT_LCD;
}

/*******************************************************************************
* Название функции   : set_pixel (private)
* Описание функции   : Функция устанавливает курсор по заданным координатам и
*                    : закрашивает один пиксель. При этом координаты курсора
*                    : НЕ сохраняются в глобальных переменных Tmp_x и Tmp_y
* Функция принимает  : unsigned int X - координата по оси X
*                    : unsigned int X - координата по оси Y
*                    : unsigned int Color - цвет
* Функция возвращает : пусто
*******************************************************************************/
void set_pixel(unsigned int X, unsigned int Y, unsigned int Color)
{
  window(X, MAX_X, Y, MAX_Y);

  WRITE_DATA;
  SELECT_LCD;

  Write_Color(Color);

  DESELECT_LCD;
}

/*******************************************************************************
* Название функции   : fill_color_TFT (public)
* Описание функции   : Закрашивает весь экран заданным цветом
* Функция принимает  : unsigned int Color - цвет
* Функция возвращает : пусто
*******************************************************************************/
void fill_color_TFT(unsigned int Color)
{
  unsigned long i;
  window(0, MAX_X, 0, MAX_Y);
  i = (unsigned long)(MAX_X+1)*(MAX_Y+1);
  i >>= 2;
  WRITE_DATA;
  SELECT_LCD;
  while(i--)
  {
    Write_Color(Color);
    Write_Color(Color);
    Write_Color(Color);
    Write_Color(Color);
  }
  DESELECT_LCD;
}

/*******************************************************************************
* Название функции   : Clear (public)
* Описание функции   : Закрашивает весь экран цветом, который был установлен в
*                    : качестве цвета фона BackColor
* Функция принимает  : пусто
* Функция возвращает : пусто
*******************************************************************************/

void Clear(void)
{
  fill_color_TFT(dispSet.bColor);
}

/*******************************************************************************
* Название функции   : fill_color_area (public)
* Описание функции   : Функция закрашивает заданным цветом область экрана в
*                    : соответствии с полученными координатами
* Функция принимает  : unsigned int XL - левая граница по оси X
*                    : unsigned int XR - правая граница по оси X
*                    : unsigned int YL - левая граница по оси Y
*                    : unsigned int YR - правая граница по оси Y
*                    : unsigned int Color - цвет
*                    :
*                    :
* Функция возвращает : пусто
*******************************************************************************/
void fill_color_area(unsigned int XL, unsigned int XR, unsigned int YL, unsigned int YR, unsigned int Color)
{
  uint8_t w, h;
  if(XL >= LCD_width || YL >= LCD_height) {
    return;
  }

  if((XR) >= LCD_width) {
    w = LCD_width  - XL + 1;
    } else {
    w = XR - XL + 1;
  }
  if((YR) >= LCD_height) {
    h = LCD_height - YL + 1;
    } else {
    h = YR - YL + 1;
  }

  window(XL, XR, YL, YR);;

  WRITE_DATA;
  SELECT_LCD;

  for(uint8_t i = 0; i < h; i++) {
    for(uint8_t j = 0; j < w; j++) {
      Write_Color(Color);
    }
  }

  DESELECT_LCD;
}


/*******************************************************************************
* Название функции   : put_char (private)
* Описание функции   : Функция выводит на дисплей один символ с прозрачным фоном
* Функция принимает  : unsigned char c - символ
*                    : unsigned int Color - цвет символа
* Функция возвращает : пусто
*******************************************************************************/
void put_char (unsigned char c, unsigned int Color)
{
	uint8_t i,ch;
	uint16_t j;
	uint16_t temp, x, y;
	if(Tmp_x > LCD_width - dispSet.x_size) {
		Tmp_x = 0;
		Tmp_y += dispSet.y_size;
#ifdef SevenSeg
		if (dispSet.font == SevenSegNumFont) {
			Tmp_y += 2;
		}
#endif	//SevenSeg
	}
	if(Tmp_y > LCD_height - dispSet.y_size) {
		Tmp_y = 0;
		Tmp_x = 0;
	}
	x = Tmp_x;
	y = Tmp_y;
	temp=((c - dispSet.offset) * ((dispSet.x_size / 8) * dispSet.y_size)) + 4;
	for(j=0;j<dispSet.y_size;j++) {
		for (int zz = 0; zz < (dispSet.x_size / 8); zz++) {
			ch=dispSet.font[temp + zz];
			for(i=0;i<8;i++) {
				if((ch & (1 << (7 - i))) != 0) {
					window(x + i + (zz * 8), x+i+(zz*8)+1, y+j, y+j+1);
					SELECT_LCD;
					WRITE_DATA;
					Write_Color(Color);
				}
			}
		}
		temp += (dispSet.x_size / 8);
	}
	DESELECT_LCD;
	window(0, MAX_X, 0, MAX_Y);
}

/*******************************************************************************
* Название функции   : put_charWBC (private)
* Описание функции   : Функция выводит один символ заданного цвета и фон закрашивает
*                    : закрашивает заданным цветом фона
* Функция принимает  : unsigned char c - символ
*                    : unsigned int Color - цвет символа
*                    : unsigned int B_color - цвет фона
* Функция возвращает : пусто
*******************************************************************************/
void put_charWBC (unsigned char c, unsigned int Color, unsigned int B_color)
{
  uint8_t i, ch;
  uint16_t temp, j;
  //Serial.print(c, HEX);
  if(Tmp_x > LCD_width - dispSet.x_size) {
    Tmp_x = 0;
    Tmp_y += dispSet.y_size;
    #ifdef SevenSeg
    if (dispSet.font == SevenSegNumFont) {
      Tmp_y += 2;
    }
    #endif	//SevenSeg
  }
  if(Tmp_y > LCD_height - dispSet.y_size) {
    Tmp_y = 0;
    Tmp_x = 0;
  }
  window(Tmp_x, Tmp_x + dispSet.x_size - 1, Tmp_y, Tmp_y + dispSet.y_size - 1);

  WRITE_DATA;
  SELECT_LCD;

  temp = ((c - dispSet.offset) * ((dispSet.x_size / 8) * dispSet.y_size)) + 4;
  for(j = 0; j < ((dispSet.x_size / 8) * dispSet.y_size); j++)
  {
    ch = dispSet.font[temp];
    for(i = 0; i < 8; i++)
    {
      if((ch & (1 << (7 - i))) != 0)
      {
	Write_Color(Color);
      }
      else
      {
	Write_Color(B_color);
      }
    }
    temp++;
  }
  DESELECT_LCD;
  window(0, MAX_X, 0, MAX_Y);
}


/*******************************************************************************
* Название функции   : Print (public)
* Описание функции   : Функция выводит строку на экран, цвет и фон строки
*                    : соответствует установленным ранее цветам текста и фона
*                    : Кроме символов шрифта функция поддерживает символы: '\r'
*                    : и '\n'
*                    : Строка выводится по заранее установленным координатам
* Функция принимает  : const char* str - указатель на строку
* Функция возвращает : пусто
*******************************************************************************/
void Print(const char* str)
{
  if(transporent) {
    put_string(str, dispSet.color);
  } else {
    put_stringWBC(str, dispSet.color, dispSet.bColor);
  }
}

/*******************************************************************************
* Название функции   : PrintXY (public)
* Описание функции   : Функция выводит строку на экран, в соответствии с
*                    : указанными координатами.
*                    : Цвет и фон строки соответствует установленным ранее
*                    : цветам текста и фона. Кроме символов шрифта функция
*                    : поддерживает символы: '\r' и '\n'
* Функция принимает  : const char* str - указатель на строку
*                    : uint16_t x - координата по оси X
*                    : uint16_t y - координата по оси Y
* Функция возвращает : пусто
*******************************************************************************/
void PrintXY(const char* str, uint16_t x, uint16_t y)
{
  cursor(x, y);
  if(transporent) {
    put_string(str, dispSet.color);
  } else {
    put_stringWBC(str, dispSet.color, dispSet.bColor);
  }
}

/*******************************************************************************
* Название функции   : PrintXYCol (public)
* Описание функции   : Функция выводит строку на экран, в соответствии с
*                    : указанными координатами и цветом символов.
*                    : Фон строки соответствует установленному ранее
*                    : цвету фона. Кроме символов шрифта функция
*                    : поддерживает символы: '\r' и '\n'
* Функция принимает  : const char* str - указатель на строку
*                    : uint16_t x - координата по оси X
*                    : uint16_t y - координата по оси Y
*                    : uint16_t Color - цвет строки
* Функция возвращает : пусто
*******************************************************************************/
void PrintXYCol(const char* str, uint16_t x, uint16_t y, uint16_t Color)
{
  cursor(x, y);
  if(transporent) {
    put_string(str, Color);
  } else {
    put_stringWBC(str, Color, dispSet.bColor);
  }
}

/*******************************************************************************
* Название функции   : PrintXYColBCol (public)
* Описание функции   : Функция выводит строку на экран, в соответствии с
*                    : указанными координатами, цветом символов и цветом
*                    : фона. Кроме символов шрифта функция
*                    : поддерживает символы: '\r' и '\n'
* Функция принимает  : const char* str - указатель на строку
*                    : uint16_t x - координата по оси X
*                    : uint16_t y - координата по оси Y
*                    : uint16_t Color - цвет строки
*                    : uint16_t bColor - цвет фона
* Функция возвращает : пусто
*******************************************************************************/
void PrintXYColBCol(const char* str, uint16_t x, uint16_t y, uint16_t Color, uint16_t bColor)
{
  cursor(x, y);
  put_stringWBC(str, Color, bColor);
}


/*******************************************************************************
* Название функции   : PrintInt (public)
* Описание функции   : Функция выводит на экран целое число размером до 32 бита
* Функция принимает  : uint32_t iNT - число для вывода на экран
* Функция возвращает : пусто
*******************************************************************************/
void PrintInt(int32_t iNT)
{
	uint32_t tmp = 10;
	uint8_t cnt = 0;
  char Str[20];
  if(iNT < 0) {
  		iNT *= -1;
  		Str[cnt++] = '-';
  }
  while(iNT >= tmp) tmp *= 10;
  tmp /= 10;
  while(tmp > 1) {
  		Str[cnt++] = iNT / tmp + 0x30;
  		iNT -= iNT / tmp * tmp;
  		tmp /= 10;
  }
  Str[cnt++] = iNT + 0x30;
  Str[cnt++] = '\0';
  transporent? put_string(Str, dispSet.color) : put_stringWBC(Str, dispSet.color, dispSet.bColor);
}


/*******************************************************************************
* Название функции   : PrintInt (public)
* Описание функции   : Функция выводит на экран целое число размером до 32 бита
* 	                   : по заданным координатам
* Функция принимает  : uint16_t x - координата по оси Х
*                    : uint16_t y - координата по оси Y
* Функция возвращает : пусто
*******************************************************************************/
void PrintIntXY(int32_t iNT, uint16_t x, uint16_t y)
{
	cursor(x, y);
	PrintInt(iNT);
}

/*******************************************************************************
* Название функции   : put_string (public)
* Описание функции   : Функция выводит строку на экран, в соответствии с
*                    : заданным цветом символов. Фон прозрачный
* Функция принимает  : const char* str - указатель на строку
*                    : unsigned int Color - цвет строки
* Функция возвращает : пусто
*******************************************************************************/
void put_string (const char * st, unsigned int Color)
{
	int stl, i;
	unsigned char temp;
	stl = strlen(st);

	for (i=0; i<stl; i++) {
		temp = *st++;
		if(temp == 0xD0) {
			//Serial.print(temp, HEX);
			temp = *st++;
			//Serial.print(temp, HEX);
			if(temp == 0x84) {	// Є
				temp += 0x3D;
			} else if (temp == 0x87) {	// Ї
				temp += 0x3B;
			} else if (temp == 0x81) {	// Ё
				temp += 0x42;
			} else {
				temp -= 17;
			}
			//Serial.print(temp, HEX);
			stl--;
		} else if (temp == 0xD1) {
			//Serial.print(temp, HEX);
			temp = *st++;
			//Serial.print(temp, HEX);
			if(temp == 0x94) {	// є
				temp += 0x2B;
			} else if (temp == 0x97) {	// ї
				temp += 0x29;
			} else if (temp == 0x91) {	// ё
				temp += 0x33;
			} else {
				temp += 0x2F;
			}
			//Serial.print(temp, HEX);
			stl--;
		}  else if (temp == 0xC2) {
			temp = *st++;
			stl--;
		}
		if (temp == '\n'){
			Tmp_x = 0;
		} else if (temp == '\r'){
			Tmp_y += dispSet.y_size;
			if(Tmp_y > (LCD_height - dispSet.x_size)) {
				Tmp_y = 0;
			}
#ifdef SevenSeg
			if (dispSet.font == SevenSegNumFont) {
				Tmp_y += 2;
			}
#endif	//SevenSeg
		} else if ((temp == 0xF8) || (temp == 0xB0)) {	// degrees Celsius sign
			uint16_t x = Tmp_x, y = Tmp_y;
			DrawCircle(Tmp_x + 3, Tmp_y + 5, 2, Color);
			Tmp_y = y;
			Tmp_x = x + dispSet.x_size;
		} else {
#ifdef SevenSeg
			if(dispSet.font == SevenSegNumFont) {
				if((temp == '.') || (temp == ',')) {
					Tmp_x += 2;
					FillCircle(Tmp_x, Tmp_y + 39, 2, Color);
					Tmp_x += 4;
				} else {
					put_char(temp, Color);
					Tmp_x += dispSet.x_size;
				}
			} else {
#endif	//SevenSeg
				put_char(temp, Color);
				Tmp_x += dispSet.x_size;
#ifdef SevenSeg
			}
#endif	//SevenSeg
		}
	}
}

/*******************************************************************************
* Название функции   : put_stringWBC (public)
* Описание функции   : Функция выводит строку на экран, в соответствии с
*                    : заданными цветами символов и фона
* Функция принимает  : const char* str - указатель на строку
*                    : unsigned int Color - цвет строки
*                    : unsigned int B_color - цвет фона
* Функция возвращает : пусто
*******************************************************************************/
void put_stringWBC (const char *st, unsigned int Color, unsigned int B_color)
{
	int stl, i;
	unsigned char temp;
	//stl = strlen(st);
	stl = 0;
	while(st[stl] != '\0') {
		stl++;
	}

	for (i=0; i<stl; i++) {
		temp = *st++;
		if(temp == 0xD0) {
			//Serial.print(temp, HEX);
			temp = *st++;
			//Serial.print(temp, HEX);
			if(temp == 0x84) {	// Є
				temp += 0x3D;
			} else if (temp == 0x87) {	// Ї
				temp += 0x3B;
			} else if (temp == 0x81) {	// Ё
				temp += 0x42;
			} else {
				temp -= 17;
			}
			//Serial.print(temp, HEX);
			stl--;
		} else if (temp == 0xD1) {
			//Serial.print(temp, HEX);
			temp = *st++;
			//Serial.print(temp, HEX);
			if(temp == 0x94) {	// є
				temp += 0x2B;
			} else if (temp == 0x97) {	// ї
				temp += 0x29;
			} else if (temp == 0x91) {	// ё
				temp += 0x33;
			} else {
				temp += 0x2F;
			}
			//Serial.print(temp, HEX);
			stl--;
		} else if (temp == 0xC2) {
			temp = *st++;
			if(temp == 0xB0) {
				temp = 0xF8;
			}
			stl--;
		}
		if (temp == '\n'){
			Tmp_x = 0;
		} else if (temp == '\r'){
			Tmp_y += dispSet.y_size;
			if(Tmp_y > (LCD_height - dispSet.x_size)) {
				Tmp_y = 0;
			}
#ifdef SevenSeg
			if (dispSet.font == SevenSegNumFont) {
				Tmp_y += 2;
			}
#endif	//SevenSeg
		} else if (temp == 0xF8) {				// degrees Celsius sign
			uint16_t x = Tmp_x, y = Tmp_y;
			put_charWBC(' ', Color, B_color);
			DrawCircle(Tmp_x + 3, Tmp_y + 5, 2, Color);
			Tmp_y = y;
			Tmp_x = x + dispSet.x_size;
		} else {
#ifdef SevenSeg
			if(dispSet.font == SevenSegNumFont) {
				if((temp == '.') || (temp == ',')) {
					Tmp_x += 2;
					FillCircle(Tmp_x, Tmp_y + 39, 2, Color);
					Tmp_x += 4;
				} else {
					put_charWBC(temp, Color, B_color);
					Tmp_x += dispSet.x_size;
				}
			} else {
#endif	//SevenSeg
				put_charWBC(temp, Color, B_color);
				Tmp_x += dispSet.x_size;
#ifdef SevenSeg
			}
#endif	//SevenSeg
		}
	}
}


#ifdef printHex
/*******************************************************************************
* Название функции   : put_hex (public)
* Описание функции   : Функция активна если раскомментирован макрос
*                    : #define printHex в файле ST7735.h
*                    : Функция выводит на дисплей четыре байта в шеснадцатиричной
*                    : системе, перед ними добавляет приставку "0x"
* Функция принимает  : uint32_t hEX - число для вывода на экран
*                    : unsigned int Color - цвет символов (фон прозрачный)
* Функция возвращает : пусто
*******************************************************************************/
void put_hex(uint32_t hEX, unsigned int Color)
{
  char Str[10];
  sprintf(Str, "0x%X ", (unsigned int)hEX);
  put_string(Str, Color);
}

/*******************************************************************************
* Название функции   : put_hexWBC (public)
* Описание функции   : Функция активна если раскомментирован макрос
*                    : #define printHex в файле ST7735.h
*                    : Функция выводит на дисплей четыре байта в шеснадцатиричной
*                    : системе, перед ними добавляет приставку "0x"
* Функция принимает  : uint32_t hEX - число для вывода на экран
*                    : unsigned int Color - цвет символов
*                    : unsigned int B_color - цвет фона
* Функция возвращает : пусто
*******************************************************************************/
void put_hexWBC(unsigned long hEX, unsigned int Color, unsigned int B_color)
{
  char Str[10];
  sprintf(Str, "0x%X ", (unsigned int)hEX);
  put_stringWBC(Str, Color, B_color);
}
#endif	// printHex


/*******************************************************************************
* Название функции   : DrawLine (public)
* Описание функции   : Функция выводит на экран динию по заданным координатам
* Функция принимает  : int x1 - начало линии, координаты по оси x
*                    : int y1 - начало линии, координаты по оси y
*                    : int x2 - конец линии, координаты по оси x
*                    : int y2 - конец линии, координаты по оси y
*                    : int color - цвет линии
* Функция возвращает : пусто
*******************************************************************************/
void DrawLine (int x1, int y1, int x2, int y2, int color)
{
  short  x, y, d, dx, dy, i, i1, i2, kx, ky;
  signed char flag;

  dx = x2 - x1;
  dy = y2 - y1;
  if (dx == 0 && dy == 0) set_pixel(x1, y1, color);  //Точка
  else      //Линия
  {
    kx = 1;
    ky = 1;
    if( dx < 0 )
    {
      dx = -dx;
      kx = -1;
    }
    else
    if(dx == 0) kx = 0;
    if(dy < 0)
    {
      dy = -dy;
      ky = -1;
    }
    if(dx < dy)
    {
      flag = 0;
      d = dx;
      dx = dy;
      dy = d;
    }
    else flag = 1;
    i1 = dy + dy;
    d = i1 - dx;
    i2 = d - dx;
    x = x1;
    y = y1;

    for(i=0; i < dx; i++)
    {
      set_pixel(x, y, color);
      if(flag) x += kx;
      else y += ky;
      if( d < 0 ) d += i1;
      else
      {
	d += i2;
	if(flag) y += ky;
	else x += kx;
      }
    }
    set_pixel(x, y, color);
  }
}

/*******************************************************************************
* Название функции   : DrawVLine (public)
* Описание функции   : Функция выводит вертикальную линию на экран
* Функция принимает  : int x - начало линии, координаты по оси x
*                    : int y - начало линии, координаты по оси y
*                    : int len - длина линии
*                    : int color - цвет линии
* Функция возвращает : пусто
*******************************************************************************/
void DrawVLine (int x, int y, int len, int color)
{
  uint8_t y2 = y + len - 1;
  window(x, x, y, y2);
  WRITE_DATA;
  SELECT_LCD;
  while(len--) {
	  Write_Color(color);
  }
  DESELECT_LCD;
  window(0, MAX_X, 0, MAX_Y);
}

/*******************************************************************************
* Название функции   : DrawHLine (public)
* Описание функции   : Функция выводит горизонтальную линию на экран
* Функция принимает  : int x - начало линии, координаты по оси x
*                    : int y - начало линии, координаты по оси y
*                    : int len - длина линии
*                    : int color - цвет линии
* Функция возвращает : пусто
*******************************************************************************/
void DrawHLine (int x, int y, int len, int color)
{
  uint8_t x2 = x + len - 1;
  window(x, x2, y, y);
  WRITE_DATA;
  SELECT_LCD;
  while(len--) {
    Write_Color(color);
  }
  DESELECT_LCD;
  window(0, MAX_X, 0, MAX_Y);
}

/*******************************************************************************
* Название функции   : DrawCircle (public)
* Описание функции   : Функция выводит на экран окружность
* Функция принимает  : int xcenter - координаты центра окружности по оси x
*                    : int ycenter - координаты центра окружности по оси y
*                    : char rad - радиус
*                    : int color - цвет окружности
* Функция возвращает : пусто
*******************************************************************************/
void DrawCircle (int xcenter, int ycenter, char rad, int color)
{
  int tswitch, x1=0, y1;
  int d;

  d = ycenter - xcenter;
  y1 = rad;
  tswitch = 3 - 2 * rad;
  while (x1 <= y1)
  {
    put_pixel(xcenter + x1, ycenter + y1, color);
    put_pixel(xcenter + x1, ycenter - y1, color);
    put_pixel(xcenter - x1, ycenter + y1, color);
    put_pixel(xcenter - x1, ycenter - y1, color);
    put_pixel(ycenter + y1 - d, ycenter + x1, color);
    put_pixel(ycenter + y1 - d, ycenter - x1, color);
    put_pixel(ycenter - y1 - d, ycenter + x1, color);
    put_pixel(ycenter - y1 - d, ycenter - x1, color);

    if (tswitch < 0) tswitch += (4 * x1 + 6);
    else
    {
      tswitch += (4 * (x1 - y1) + 10);
      y1--;
    }
    x1++;
  }
}

/*******************************************************************************
* Название функции   : FillCircle (public)
* Описание функции   : Функция закрашивает круг заданным цветом
* Функция принимает  : int xcenter - координаты центра круга по оси x
*                    : int ycenter - координаты центра круга по оси y
*                    : char rad - радиус
*                    : int color - цвет круга
* Функция возвращает : пусто
*******************************************************************************/
void FillCircle (int xcenter, int ycenter, char rad, int color)
{
  signed int x1=0, y1, tswitch;
  y1 = rad;
  tswitch = 1 - rad;

  do
  {
    DrawLine(xcenter-x1, ycenter+y1, xcenter+x1, ycenter+y1, color);
    DrawLine(xcenter-x1, ycenter-y1, xcenter+x1, ycenter-y1, color);
    DrawLine(xcenter-y1, ycenter+x1, xcenter+y1, ycenter+x1, color);
    DrawLine(xcenter-y1, ycenter-x1, xcenter+y1, ycenter-x1, color);

    if(tswitch < 0)
    tswitch+= 3 + 2*x1++;
    else
    tswitch+= 5 + 2*(x1++ - y1--);
  } while(x1 <= y1);
}

// Нарисовать рамку
/*******************************************************************************
* Название функции   : DrawRect (public)
* Описание функции   : Функция рисует на экране прямоугольник
* Функция принимает  : int x1 - координаты верхнего левого угла по оси x
*                    : int y1 - координаты верхнего левого угла по оси y
*                    : int width - длина
*                    : int height - высота
*                    : char size - толщина линии
*                    : int color - цвет линии
* Функция возвращает : пусто
*******************************************************************************/
void DrawRect (int x1, int y1, int width, int height, char size, int color)
{
  unsigned int i;
  int x2=x1+(width-1), y2=y1+(height-1); //Конечные размеры рамки по осям х и у
  for( i=1; i<=size; i++)   // size - толщина рамки
  {
    DrawLine(x1, y1, x1, y2, color);
    DrawLine(x2, y1, x2, y2, color);
    DrawLine(x1, y1, x2, y1, color);
    DrawLine(x1, y2, x2, y2, color);
    x1++; // Увеличиваю толщину рамки, если это задано
    y1++;
    x2--;
    y2--;
  }
}


// Залить рамку цветом
/*******************************************************************************
* Название функции   : FillRect (public)
* Описание функции   : Функция заливает заданным цветом прямоугольник
* Функция принимает  : int x1 - координаты верхнего левого угла по оси x
*                    : int y1 - координаты верхнего левого угла по оси y
*                    : int width - длина
*                    : int height - высота
*                    : int color - цвет
* Функция возвращает : пусто
*******************************************************************************/
void FillRect (int x1, int y1, int width, int height, int color)
{
  fill_color_area(x1, x1+(width-1), y1, y1+(height-1),color);
  window(0,MAX_X,0,MAX_Y);
}



/*******************************************************************************
* Название функции   : DrawTriangle (public)
* Описание функции   : Функция отрисовывает на экране трехугольник
* Функция принимает  : int x1 - координата первого угла по оси x
*                    : int y1 - координата первого угла по оси y
*                    : int x2 - координата второго угла по оси x
*                    : int y2 - координата второго угла по оси y
*                    : int x3 - координата третьего угла по оси x
*                    : int y3 - координата третьего угла по оси y
*                    : int color - цвет линий
* Функция возвращает : пусто
*******************************************************************************/
void DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int color)
{
  DrawLine(x1, y1, x2, y2, color);
  DrawLine(x3, y3, x1, y1, color);
  DrawLine(x3, y3, x2, y2, color);
}



/*******************************************************************************
* Название функции   : FillTriangleA (private)
* Описание функции   : Служебная функция для закрашивания терхугольника
* Функция принимает  : int x1 - координата первого угла по оси x
*                    : int y1 - координата первого угла по оси y
*                    : int x2 - координата второго угла по оси x
*                    : int y2 - координата второго угла по оси y
*                    : int x3 - координата третьего угла по оси x
*                    : int y3 - координата третьего угла по оси y
*                    : int color - цвет
* Функция возвращает : пусто
*******************************************************************************/
void FillTriangleA(int x1, int y1, int x2, int y2, int x3, int y3, int color)
{
  signed long x, y, addx, dx, dy;
  signed long P;
  int i;
  long a1,a2,b1,b2;
  if(y1>y2)  {b1=y2; b2=y1; a1=x2; a2=x1;}
  else       {b1=y1; b2=y2; a1=x1; a2=x2;}
  dx = a2 -a1;
  dy = b2 - b1;
  if(dx<0)dx=-dx;
  if(dy<0)dy=-dy;
  x = a1;
  y = b1;

  if(a1 > a2)    addx = -1;
  else           addx = 1;

  if(dx >= dy)
  {
    P = 2*dy - dx;
    for(i=0; i<=dx; ++i)
    {
      DrawLine((int)x, (int)y, x3, y3, color);
      if(P < 0)
      {
	P += 2*dy;
	x += addx;
      }
      else
      {
	P += 2*dy - 2*dx;
	x += addx;
	y ++;
      }
    }
  }
  else
  {
    P = 2*dx - dy;
    for(i=0; i<=dy; ++i)
    {
      DrawLine((int)x, (int)y, x3, y3, color);
      if(P < 0)
      {
	P += 2*dx;
	y ++;
      }
      else
      {
	P += 2*dx - 2*dy;
	x += addx;
	y ++;
      }
    }
  }
}

// Залить треугольник цветом
/*******************************************************************************
* Название функции   : FillTriangle (public)
* Описание функции   : Функция закрашивает трехугольник
* Функция принимает  : int x1 - координата первого угла по оси x
*                    : int y1 - координата первого угла по оси y
*                    : int x2 - координата второго угла по оси x
*                    : int y2 - координата второго угла по оси y
*                    : int x3 - координата третьего угла по оси x
*                    : int y3 - координата третьего угла по оси y
*                    : int color - цвет
* Функция возвращает : пусто
*******************************************************************************/
void FillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int color)
{
  FillTriangleA(x1, y1, x2, y2, x3, y3, color);
  FillTriangleA(x3, y3, x1, y1, x2, y2, color);
  FillTriangleA(x3, y3, x2, y2, x1, y1, color);
}

/*******************************************************************************
* Название функции   : DrawBitmap (public)
* Описание функции   : Функция выводит на экран цветное изображение из массива
*                    : const unsigned char записанного во flash память программы
*                    : Глубина цвета 16 бит. Для перекодирования изображения в
*                    : массив подходит программа Image2Lcd. При использовании
*                    : других програм важно соблюдать условия:
*                    : 1. первые два байта массива после "шапки" - это верхний
*                    : левый пиксель изображения, последние два - правый нижний.
*                    : Сканирование слева направо, сверху вниз. Массив дожен
*                    : содержать только однобайтные данные
*                    : 2. Первые 8 байт массива - это шапка, в ней хранится
*                    : следующая информация:
*                    : байт 0  - тип сканирования, должен быть всегда 0
*                    : байт 1 - глубина цвета, должен быть 0x10
*                    : байт 2 - ширина изображения
*                    : байт 4 - высота изображения
*                    : Остальные байты шапки функция игнорирует
* Функция принимает  : uint8_t x - верхний левый угол, координаты по оси x
*                    : uint8_t y - верхний левый угол, координаты по оси y
*                    : PGM_P bitmap - указатель на массив данных об изображении
* Функция возвращает : пусто
*******************************************************************************/
void DrawBitmap(uint8_t x, uint8_t y, const unsigned char *bitmap) {
  bitmap += 2;
  uint8_t w = bitmap[0];
  bitmap += 2;
  uint8_t h = bitmap[0];
  bitmap += 4;
  uint8_t max_x = x + w - 1;
  uint8_t max_y = y + h - 1;

  if(x >= LCD_width || y >= LCD_height) {
    return;
  }

  if(max_x >= LCD_width) {
    max_x = LCD_width - 1;
  }

  if(max_y >= LCD_height) {
    max_y = LCD_height - 1;
  }

  window(x, max_x, y, max_y);

  WRITE_DATA;
  SELECT_LCD;

  for(uint8_t i = 0; i < h; i++) {
    for(uint8_t j = 0; j < w; j++) {
      if((x + j) >= LCD_width || (y + i) >= LCD_height) {
	bitmap += 2;
	continue;
      }
      uint8_t color = bitmap[1];
      spiTransmite(color);
      color = bitmap[0];
      spiTransmite(color);
      bitmap += 2;
    }
  }
  DESELECT_LCD;
}

/*******************************************************************************
* Название функции   : DrawMonoBitmap (public)
* Описание функции   : Функция выводит на экран монохромное изображение
*                    : заданным цветом и закрашивает фон другим заданным цветом.
*                    : Для перекодирования изображения в массив подходит
*                    : программа Image2Lcd. При использовании других програм
*                    : важно соблюдать условия:
*                    : Первых шесть байт массива - это шапка. Информация о
*                    : пикселях хранится в однобайтных переменных, но по одному
*                    : биту на пиксель: 0 - не закрашен, 1 - закрашен. Если ширина
*                    : изображения не кратна восьми - последний байт каждой
*                    : строки содержит поля, сканирование слева направо, сверху вниз.
*                    : Шапка содержит следующую информацию:
*                    : байт 0  - тип сканирования, должен быть всегда 0
*                    : байт 1 - глубина цвета, должен быть 0x01
*                    : байт 2 - ширина изображения
*                    : байт 4 - высота изображения
*                    : Остальные байты шапки функция игнорирует
*                    : Также функция поддерживает сканирование слево направо, но
*                    : столбцами по восемь пикселей. Первый байт после шапки - это
*                    : первые 8 пикселей сверху вниз слева, второй байт - это следующие
*                    : 8 пикселей расположенные вертикально, но не под первыми, а справа
*                    : от них. В таком случае тип сканирования (байт 0 шапки) будет 0x02
*                    : Такой метод удобен для монохромных дисплеев, так как там экран
*                    : закрашивается не попиксельно, а побайтно. На цветном экране
*                    : изображение сохранённое в такой массив отрисовывается дольше,
*                    : чем при горизонтальном сканировании, поэтому его использовать
*                    : не рекомендую
*                    : Если в функцию передать цветное изображение с глубиной цвета
*                    : 16 бит - она его распознает и сама переправит функции
*                    : DrawBitmap(), которая выведет его на экран, поэтому если нужно
*                    : выводить периодически разные картинки, то цветные, то монохромные
*                    : - удобно все их передавать этой функци, а она разберётся сама,
*                    : что с каким изображением делать.
*                    :
*                    :
*                    :
* Функция принимает  : uint8_t x - верхний левый угол, координаты по оси x
*                    : uint8_t y - верхний левый угол, координаты по оси y
*                    : PGM_P bitmap - указатель на массив данных об изображении
*                    : uint16_t color_set - цвет пикселя
*                    : uint16_t color_unset - цвет фона
* Функция возвращает : пусто
*******************************************************************************/
void DrawMonoBitmap(uint8_t x, uint8_t y, const unsigned char *bitmap, uint16_t color_set, uint16_t color_unset) {
  unsigned int _x, _y;
  unsigned char i, tmp;
  if (bitmap[1] != 0x01) {	// если изображение не монохромное
    if ((bitmap[1] == 0x10) && (bitmap[0] == 0x00)) {
      // если глубина цвета 16 бит и сканирование горизонтальное слева направо
      // передать соответствующей функции
      DrawBitmap(x, y, bitmap);
    } else {
      return;
    }
  } else if (bitmap[0] == 0x02) {
    // если монохромное и сканирование "Data hor, Byte ver" - вывести на экран
    unsigned int xend = x + bitmap[2];
    unsigned int yend = y + bitmap[4];
    bitmap += 6;
    _x = x;
    _y = y;
    unsigned long C = 1;
    while(C)
    {
      tmp = bitmap[0];
      bitmap++;
      for(i = 0; i < 8; i++) {
	if(tmp & 0x80) {
	  put_pixel(_x, _y, color_set);
	    } else {
	  put_pixel(_x, _y, color_unset);
	}
	_y++;
	tmp <<= 1;
      }
      _x++;
      if(_x == xend) {
	_x = x;
	if(_y >= yend) {
	  C = 0;
	}
	} else {
	_y -= 8;
      }
    }
    window(0, MAX_X, 0, MAX_Y);
    } else if (bitmap[0] == 0x00) {
    // если монохромное и сканирование "Horizon Scan" - вывести на экран
    uint8_t w = bitmap[2];
    uint8_t h = bitmap[4];
    uint8_t max_x = x + w - 1;
    uint8_t max_y = y + h - 1;
    bitmap += 6;
    if(x >= LCD_width || y >= LCD_height) {
      return;
    }

    window(x, max_x, y, max_y);

    WRITE_DATA;
    SELECT_LCD;

    uint8_t bit_pos = 0;
    uint8_t byte = 0;
    uint8_t tmp_x = x;
    for(uint8_t i = 0; i < h; i++) {
      for(uint8_t j = 0; j < w; j++) {
	if(bit_pos % 8 == 0) {
	  byte = bitmap[0];
	  bitmap++;
	  bit_pos = 0;
	}
	if(byte & (0x80 >> bit_pos)) {
	  Write_Color(color_set);
	  } else {
	  Write_Color(color_unset);
	}
	bit_pos++;
	tmp_x++;
	if(tmp_x > max_x) {
	  bit_pos = 8;
	  tmp_x = x;
	}
      }
    }
    DESELECT_LCD;
  }
}

