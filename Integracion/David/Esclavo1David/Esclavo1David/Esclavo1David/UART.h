/*
 * UART.h
 *
 * Created: 12/02/2026 08:33:05 p. m.
 *  Author: Admin
 */ 


#ifndef UART_H_
#define UART_H_

#include <stdint.h>

/************************************************************************/
/* CONFIGURACIÓN                                                        */
/************************************************************************/

typedef enum
{
	UART_INTERRUPTS_DISABLED = 0,
	UART_INTERRUPTS_ENABLED  = 1
} UART_INTERRUPT_CONFIG;

typedef enum
{
	UART_BAUD_4800_16MHZ    = 416,
	UART_BAUD_9600_16MHZ    = 208,
	UART_BAUD_19200_16MHZ   = 104,
	UART_BAUD_38400_16MHZ   = 52,
	UART_BAUD_57600_16MHZ   = 34,
	UART_BAUD_115200_16MHZ  = 17
} UART_BAUD_CONFIG;

/************************************************************************/
/* FUNCIONES                                                            */
/************************************************************************/

void UART_Init(UART_BAUD_CONFIG ubrr_value,
UART_INTERRUPT_CONFIG interrupts_en);

void UART_Init_9600(void);

void UART_SendChar(char c);
void UART_SendString(const char* str);

uint8_t UART_DataAvailable(void);
uint8_t UART_ReadChar(void);

#endif /* UART_H_ */