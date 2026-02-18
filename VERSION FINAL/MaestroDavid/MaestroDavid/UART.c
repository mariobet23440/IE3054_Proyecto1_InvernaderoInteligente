/*
 * UART.c
 *
 * Created: 12/02/2026 08:33:22 p. m.
 *  Author: Admin
 */ 
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include "UART.h"

/************************************************************************/
/* INICIALIZACIÓN                                                       */
/************************************************************************/

void UART_Init(UART_BAUD_CONFIG ubrr_value,
UART_INTERRUPT_CONFIG interrupts_en)
{
	// Configurar pines
	DDRD |=  (1 << DDD1);   // TX salida
	DDRD &= ~(1 << DDD0);   // RX entrada

	// Reset registros
	UCSR0A = 0;
	UCSR0B = 0;
	UCSR0C = 0;

	// Activar double speed (U2X0)
	UCSR0A |= (1 << U2X0);

	// Configurar baud rate
	UBRR0 = ubrr_value;

	// 8 bits, 1 stop, sin paridad (8N1)
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	// Habilitar RX y TX
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);

	// Habilitar interrupción RX si se solicita
	if(interrupts_en == UART_INTERRUPTS_ENABLED)
	{
		UCSR0B |= (1 << RXCIE0);
	}
}

void UART_Init_9600(void)
{
	UART_Init(UART_BAUD_9600_16MHZ, UART_INTERRUPTS_DISABLED);
}

/************************************************************************/
/* TRANSMISIÓN                                                          */
/************************************************************************/

void UART_SendChar(char c)
{
	while (!(UCSR0A & (1 << UDRE0)));   // Esperar buffer libre
	UDR0 = c;
}

void UART_SendString(const char* str)
{
	while (*str)
	{
		UART_SendChar(*str++);
	}
}

/************************************************************************/
/* RECEPCIÓN (SIN INTERRUPCIÓN)                                         */
/************************************************************************/

uint8_t UART_DataAvailable(void)
{
	return (UCSR0A & (1 << RXC0));
}

uint8_t UART_ReadChar(void)
{
	while (!(UCSR0A & (1 << RXC0)));
	return UDR0;
}
