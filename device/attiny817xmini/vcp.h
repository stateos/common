#ifndef __VCP_H__
#define __VCP_H__

// txd: PORTB.2
// rxd: PORTB.3

#include <avr/io.h>

#define USART_BAUD_RATE (F_CPU * 4ULL / 9600)

static inline
void vcp_init( void )
{
	PORTB.DIRSET |= (uint8_t) PIN2_bm;
	PORTB.DIRSET &= (uint8_t)~PIN3_bm;
	USART0.BAUD  = USART_BAUD_RATE;
	USART0.CTRLC = USART_CHSIZE_8BIT_gc;
	USART0.CTRLB = USART_RXMODE_NORMAL_gc // normal mode
	             | USART_RXEN_bm          // enable receiver
	             | USART_TXEN_bm;         // enable transmitter
}

static inline
void vcp_put( char data )
{
//	while ((USART0.STATUS & USART_TXCIF_bm) == 0); // wait if data is not transmitted
	while ((USART0.STATUS & USART_DREIF_bm) == 0); // wait if transmit buffer is full
	USART0.TXDATAL = (register8_t) data;
}

static inline
char vcp_get( void )
{
	while ((USART0.STATUS & USART_RXCIF_bm) == 0); // wait if the receive buffer is empty
	return (char) USART0.RXDATAL;
}

static inline
void vcp_puts( const char *msg )
{
	for (int i = 0; msg[i] != 0; i++)
		vcp_put(msg[i]);
}

static inline
void vcp_putd( uint32_t x )
{
	static char dec[] = "0123456789";
	if (x >= 10) vcp_putd(x / 10);
	vcp_put(dec[x % 10]);
}

void vcp_putx( uint32_t x )
{
	static char hex[] = "0123456789ABCDEF";
	if (x >= 16) vcp_putx(x / 16);
	vcp_put(hex[x % 16]);
}

#endif//__VCP_H__
