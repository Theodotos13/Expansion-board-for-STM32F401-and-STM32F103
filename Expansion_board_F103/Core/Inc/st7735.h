#ifndef _ST7735_H_
#define _ST7735_H_

#include "main.h"
#include "stdbool.h"

typedef struct  //Структура с данными энкодера
{
	SPI_HandleTypeDef *hspi;	// указатель на структуру SPI

	GPIO_TypeDef *port;  //Порт, к которому подключены CS, RS & RST

	uint16_t pin_cs;  //Пин подключения cs
	uint16_t pin_rs;  //Пин подключения rs
	uint16_t pin_rst;  //Пин подключения rst
} ST7735struct;


/*
	Для экономии памяти можно отключить возможность использования следующих функций и кириллицы
*/
#define SevenSeg	// использование семисегментного шрифта 23х41
#define printHex	// набор функций для упрощённого вывода чисел в шестнадцатиричной системе

#define Cyrillic
#define SELECT_LCD	HAL_GPIO_WritePin(st7735.port, st7735.pin_cs, GPIO_PIN_RESET);
#define DESELECT_LCD	HAL_GPIO_WritePin(st7735.port, st7735.pin_cs, GPIO_PIN_SET);

#define LCD_RES_LO	HAL_GPIO_WritePin(st7735.port, st7735.pin_rst, GPIO_PIN_RESET);
#define LCD_RES_HI	HAL_GPIO_WritePin(st7735.port, st7735.pin_rst, GPIO_PIN_SET);

#define WRITE_DATA	HAL_GPIO_WritePin(st7735.port, st7735.pin_rs, GPIO_PIN_SET);
#define WRITE_CMD	HAL_GPIO_WritePin(st7735.port, st7735.pin_rs, GPIO_PIN_RESET);

/*
	Прозрачная/непрозрачная печать
*/
#define TRANSPORENT_ON	true
#define TRANSPORENT_OFF	false

#define _delay_ms	HAL_Delay

/*
	Формат передачи цвета дисплею (RGB или BGR)
*/
#define RGB 0
#define BGR 8

/*
	размер дисплея: 128х128 или 128х160
*/
#define LCD128X128	true
#define LCD128X160	false

/*******************************************************************************
*                              Основные цвета
*******************************************************************************/
#define RED	    0xF800
#define GREEN       0x07E0
#define GARK_GREEN  0x2589
#define BLUE        0x001f
#define BLACK       0x0000
#define YELLOW      0xffe0
#define WHITE       0xffff
#define CYAN        0x07ff
#define BRIGHT_RED  0xf810
#define VIOLET      0xC318
#define GRAY        0xC618
#define GRAY1       0x18C3
#define GRAY2	    0x8410
#define BROWN       0x59A4
#define TURQUOISE   0x2DFF
#define MAGENTA	    0xF81F


// typedef enum { false, true } bool;

// ST7735 commands
enum ST7735_COMMANDS {
  ST7735_NOP = 0x00,
  ST7735_SWRESET = 0x01,
  ST7735_RDDID = 0x04,
  ST7735_RDDST = 0x09,

  ST7735_SLPIN = 0x10,
  ST7735_SLPOUT = 0x11,
  ST7735_PTLON = 0x12,
  ST7735_NORON = 0x13,

  ST7735_INVOFF = 0x20,
  ST7735_INVON = 0x21,
  ST7735_DISPOFF = 0x28,
  ST7735_DISPON = 0x29,
  ST7735_CASET = 0x2A,
  ST7735_RASET = 0x2B,
  ST7735_RAMWR = 0x2C,
  ST7735_RAMRD = 0x2E,

  ST7735_PTLAR = 0x30,
  ST7735_COLMOD = 0x3A,
  ST7735_MADCTL = 0x36,

  ST7735_FRMCTR1 = 0xB1,
  ST7735_FRMCTR2 = 0xB2,
  ST7735_FRMCTR3 = 0xB3,
  ST7735_INVCTR = 0xB4,
  ST7735_DISSET5 = 0xB6,

  ST7735_PWCTR1 = 0xC0,
  ST7735_PWCTR2 = 0xC1,
  ST7735_PWCTR3 = 0xC2,
  ST7735_PWCTR4 = 0xC3,
  ST7735_PWCTR5 = 0xC4,
  ST7735_VMCTR1 = 0xC5,

  ST7735_RDID1 = 0xDA,
  ST7735_RDID2 = 0xDB,
  ST7735_RDID3 = 0xDC,
  ST7735_RDID4 = 0xDD,

  ST7735_PWCTR6 = 0xFC,

  ST7735_GMCTRP1 = 0xE0,
  ST7735_GMCTRN1 = 0xE1
};

enum ST7735_ORIENTATION {
  LANDSCAPE,
  PORTRAIT,
  LANDSCAPE_INV,
  PORTRAIT_INV
};

enum ST7735_MADCTL_ARGS {
  MADCTL_MY = 0x80,	// Mirror Y
  MADCTL_MX = 0x40,	// Mirrror x
  MADCTL_MV = 0x20,	// Swap XY
  MADCTL_ML = 0x10,	// Scan address order
  MADCTL_RGB = 0x00,
  MADCTL_BGR = 0x08,
  MADCTL_MH = 0x04 	// Horizontal scan oder
};

typedef struct
{
  uint16_t color;
  uint16_t bColor;
  const unsigned char* font;
  uint8_t x_size;
  uint8_t y_size;
  uint8_t offset;
  uint8_t numchars;
}DisplaySettings;


extern const unsigned char  font_8x16[];
extern const unsigned char  SevenSegNumFont[];

/* Функции ------------------------------------------------------------------ */
  void LCD(SPI_HandleTypeDef *hspi, GPIO_TypeDef *port, uint16_t pin_cs, uint16_t pin_rs, uint16_t pin_rst);
  void Init (bool Size, unsigned char RGB_SET);
  void SetOrientation(unsigned char orient);
  void SetColor (unsigned int color);
  void SetOffset (signed char offsX, signed char offsY);
  void SetBackColor (unsigned int color);
  void SetFont (const unsigned char* font);
  void SetTransporent (bool Transporent);
  void SetDisplay (unsigned int color, unsigned int bColor, const unsigned char* font, bool Transporent);
  void cursor (unsigned int x, unsigned int y);
  void cursor_txt (unsigned int x, unsigned int y);
  void fill_color_TFT(unsigned int Color);
  void Clear(void);
  void fill_color_area(unsigned int XL, unsigned int XR, unsigned int YL, unsigned int YR, unsigned int Color);
  void put_string (const char * st, unsigned int Color);
  void put_stringWBC (const char *st, unsigned int Color, unsigned int B_color);
  void Print(const char* str);
  void PrintXY(const char* str, uint16_t x, uint16_t y);
  void PrintXYCol(const char* str, uint16_t x, uint16_t y, uint16_t Color);
  void PrintXYColBCol(const char* str, uint16_t x, uint16_t y, uint16_t Color, uint16_t bColor);
  void PrintInt(int32_t iNT);
  void PrintIntXY(int32_t iNT, uint16_t x, uint16_t y);
#ifdef printHex
  void put_hex(uint32_t hEX, unsigned int Color);
  void put_hexWBC(unsigned long hEX, unsigned int Color, unsigned int B_color);
#endif	// printHex
  void DrawLine (int x1, int y1, int x2, int y2, int color);
  void DrawVLine (int x, int y, int len, int color);
  void DrawHLine (int x, int y, int len, int color);
  void DrawCircle (int xcenter, int ycenter, char rad, int color);
  void FillCircle (int xcenter, int ycenter, char rad, int color);
  void DrawRect (int x1, int y1, int width, int height, char size, int color);
  void FillRect (int x1, int y1, int width, int height, int color);
  void DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int color);
  void FillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int color);
  void DrawBitmap(uint8_t x, uint8_t y, const unsigned char *bitmap);
  void DrawMonoBitmap(uint8_t x, uint8_t y, const unsigned char *bitmap, uint16_t color_set, uint16_t color_unset);
  void window(unsigned int XL, unsigned int XR, unsigned int YL, unsigned int YR);
  void put_pixel(unsigned int X, unsigned int Y, unsigned int Color);

  void spiTransmite (unsigned char data);
  void Write_Color(uint16_t color);
  void LCD_DATA (unsigned char dat);
  void LCD_CMD (unsigned char cmd);
  void set_pixel(unsigned int X, unsigned int Y, unsigned int Color);
  void FillTriangleA(int x1, int y1, int x2, int y2, int x3, int y3, int color);
  void put_char (unsigned char c, unsigned int Color);
  void put_charWBC (unsigned char c, unsigned int Color, unsigned int B_color);

#endif
