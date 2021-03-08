/******************************************************************************
 * @file    stm32l_discovery_lcd.c
 * @author  Rajmund Szymanski
 * @date    05.01.2021
 * @brief   This file contains definitions for STM32L-Discovery Kit.
 ******************************************************************************/

#include <assert.h>
#include "stm32l_discovery_lcd.h"

/* -------------------------------------------------------------------------- */

static const int seg[24] = { 0, 1, 2, 7, 8, 9, 10, 11, 12, 13, 14, 15, 17, 16, 18, 19, 20, 21, 24, 25, 26, 27, 28, 29 };

static const unsigned symbol[256] =
{
0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
0x0000,0,     0x0044,0,     0,     0,     0,     0x0040,0x0041,0x0088,0xA0DD,0xA014,0x0080,0xA000,0x0002,0x00C0, // !"#$%&'()*+,-./
0x5F00,0x4200,0xF500,0x6700,0xEA00,0xAF00,0xBF00,0x4600,0xFF00,0xEF00,0x0020,0,     0,     0,     0,     0,      //0123456789:;<=>?
0     ,0xFE00,0x6714,0x1D00,0x4714,0x9D00,0x9C00,0x3F00,0xFA00,0x0014,0x5300,0x9841,0x1900,0x5A48,0x5A09,0x5F00, //@ABCDEFGHIJKLMNO
0xFC00,0x5F01,0xFC01,0xAF00,0x0414,0x5b00,0x18C0,0x5A81,0x00C9,0x0058,0x05C0,0,     0x0009,0,     0xEC00,0x0100, //PQRSTUVWXYZ[\]^_
0x0008,0x9101,0xBB00,0xB100,0xF300,0x9180,0xA050,0x2310,0xBA00,0x9100,0x2300,0,     0x0014,0xB210,0xB200,0xB300, //`abcdefghijklmno
0,     0,     0xB000,0x2101,0xA014,0x1300,0,     0x1281,0,     0x0310,0x8180,0,     0,     0,     0,     0xFFDD, //pqrstuvwxyz{|}~ 
0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
};

/* -------------------------------------------------------------------------- */

static void LCD_Update()
{
	WRITE_REG(LCD->CLR, LCD_CLR_UDDC);
	WRITE_REG(LCD->SR,  LCD_SR_UDR);
	while (READ_BIT(LCD->SR, LCD_SR_UDD) == 0);
}

/* -------------------------------------------------------------------------- */

void LCD_Init( void )
{
	int i;

	GPIO_Init(GPIOA, GPIO_Pins(1,2,3,8,9,10,15),             GPIO_Alternate_PushPull_VeryHighSpeed | GPIO_AF11_LCD);
	GPIO_Init(GPIOB, GPIO_Pins(3,4,5,8,9,10,11,12,13,14,15), GPIO_Alternate_PushPull_VeryHighSpeed | GPIO_AF11_LCD);
	GPIO_Init(GPIOC, GPIO_Pins(0,1,2,3,6,7,8,9,10,11),       GPIO_Alternate_PushPull_VeryHighSpeed | GPIO_AF11_LCD);

	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN); READ_REG(RCC->APB1ENR);

	SET_BIT(PWR->CR,  PWR_CR_DBP);     while (READ_BIT(PWR->CR,  PWR_CR_DBP) == 0);
	SET_BIT(RCC->CSR, RCC_CSR_RTCRST);       CLEAR_BIT(RCC->CSR, RCC_CSR_RTCRST);
	SET_BIT(RCC->CSR, RCC_CSR_LSEON);  while (READ_BIT(RCC->CSR, RCC_CSR_LSERDY) == 0);
	SET_BIT(RCC->CSR, RCC_CSR_RTCSEL_LSE | RCC_CSR_RTCEN);

	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_LCDEN); READ_REG(RCC->APB1ENR);

	for (i = 0; i < 16; ++i)
		WRITE_REG(LCD->RAM[i], 0);
	WRITE_REG(LCD->SR,  LCD_SR_UDR);

	WRITE_REG(LCD->FCR, LCD_FCR_DIV | LCD_FCR_CC_0 | LCD_FCR_CC_2 | LCD_FCR_PON_2 | LCD_FCR_HD); while (READ_BIT(LCD->SR, LCD_SR_FCRSR) == 0);
	WRITE_REG(LCD->CR,  LCD_CR_DUTY_0 | LCD_CR_DUTY_1 | LCD_CR_BIAS_1 | LCD_CR_MUX_SEG | LCD_CR_LCDEN);
}

/* -------------------------------------------------------------------------- */

void LCD_Clear()
{
	int i;
	while (READ_BIT(LCD->SR, LCD_SR_UDR) != 0);
	for (i = 0; i < 16; ++i)
		WRITE_REG(LCD->RAM[i], 0);
	LCD_Update();
}

/* -------------------------------------------------------------------------- */

static void LCD_Out( int c, int n, unsigned v )
{
	unsigned b = 1U << seg[n];

	assert(c<8);
	assert(n<6);

	if (v != 0) SET_BIT(LCD->RAM[c], b);
	else      CLEAR_BIT(LCD->RAM[c], b);
}

/* -------------------------------------------------------------------------- */

static void LCD_Write( int n, unsigned v )
{
	int com;

	assert(n<6);

	while (READ_BIT(LCD->SR, LCD_SR_UDR) != 0);
	for (com = 6; com >= 0; com -= 2, v >>= 4)
	{
		LCD_Out(com, 22 - n * 2, v & 4);
		LCD_Out(com, 23 - n * 2, v & 8);
		LCD_Out(com,  0 + n * 2, v & 1);
		if (com >= 4 && n >= 4)
			continue;
		LCD_Out(com,  1 + n * 2, v & 2);
	}
	LCD_Update();
}

/* -------------------------------------------------------------------------- */

void LCD_Put( int n, int c, bool dot, bool dbl )
{
	unsigned v = symbol[c];

	assert(n<6);

	if (dot) v |= symbol['.'];
	if (dbl) v |= symbol[':'];
	LCD_Write(n, v);
}

/* -------------------------------------------------------------------------- */

void LCD_Print( char *txt )
{
	int i = 0;
	unsigned v;
	while (v = 0, i < 6)
	{
		if (*txt != 0)
		{
			v = symbol[(int)*txt++];
			while (*txt == '.' || *txt == ':')
				v |= symbol[(int)*txt++];
		}
		LCD_Write(i++, v);
	}
}

/* -------------------------------------------------------------------------- */

void LCD_Bar( unsigned v )
{
	while (READ_BIT(LCD->SR, LCD_SR_UDR) != 0);
	LCD_Out(6, 1 + 5 * 2, v & 1);
	LCD_Out(4, 1 + 5 * 2, v & 2);
	LCD_Out(6, 1 + 4 * 2, v & 4);
	LCD_Out(4, 1 + 4 * 2, v & 8);
	LCD_Update();
}

/* -------------------------------------------------------------------------- */

void LCD_Tick()
{
	static unsigned b = 0x11111111;

	LCD_Bar(b);
	b = (b << 1) | (b >> 31);
}

/* -------------------------------------------------------------------------- */
