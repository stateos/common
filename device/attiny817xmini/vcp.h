#ifndef __VCP_H__
#define __VCP_H__

// txd: PORTB.2
// rxd: PORTB.3

#include <avr/io.h>

#define USART_BAUD_RATE ((float) F_CPU * 64 / 16 / 9600)

static inline
void vcp_init( void )
{
	USART0.BAUD  = USART_BAUD_RATE;
	USART0.CTRLB = USART_RXMODE_NORMAL_gc // normal mode
	             | USART_TXEN_bm;         // enable transmitter
}

static inline
void vcp_put( char data )
{
//	while ((USART0.STATUS & USART_TXCIF_bm) == 0); // wait if data is not transmitted
	while ((USART0.STATUS & USART_DREIF_bm) == 0); // wait if transmit buffer is full
	USART0.TXDATAL = data;
}

static inline
char vcp_get( void )
{
	while ((USART0.STATUS & USART_RXCIF_bm) == 0); // wait if the receive buffer is empty
	return USART0.RXDATAL;
}

#endif//__VCP_H__
