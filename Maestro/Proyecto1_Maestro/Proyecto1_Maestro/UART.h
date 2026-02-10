/*
 * USART.h
 *
 * Created: 27/01/2026 07:10:31
 *  Author: mario
 */ 


#ifndef UART_H_
#define UART_H_
#include <stdint.h>
#include <stdlib.h>

typedef enum
{
	UART_INTERRUPTS_DISABLED = 0,
	UART_INTERRUPTS_ENABLED	= 1
} UART_INTERRUPT_CONFIG;

// Valores de UBRR presets (8 bits, sin paridad, 1 bit stop)
// Todos con double speed (Error reducido)
typedef enum
{
	UART_BAUD_4800_16MHZ	= 416,	// 0x01A0
	UART_BAUD_9600_16MHZ	= 208,	// 0x00D0
	UART_BAUD_19200_16MHZ	= 104,	// 0x0068
	UART_BAUD_38400_16MHZ	= 52,	// 0x0034
	UART_BAUD_57600_16MHZ	= 34,   // 0x0022
	UART_BAUD_115200_16MHZ	= 17	// 0x0011
} UART_BAUD_CONFIG;

void UART_Init(UART_BAUD_CONFIG ubrr_value, UART_INTERRUPT_CONFIG interrupts_en);
void UART_Init_9600(void);
void UART_sendChar(char c);
void UART_sendString(const char* str);
unsigned char UART_Receive(void);

#endif /* UART_H_ */