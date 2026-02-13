/*
 * Proyecto1_Maestro.c
 *
 * Created: 10/02/2026 14:57:37
 * Author : mario
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "HD44780_4b.h"
#include "UART.h"

/************************************************************************/
/* DEFINICIONES                                                         */
/************************************************************************/
#define UART_CMD1 'A'
#define UART_CMD2 'B'
#define UART_CMD3 'B'
#define UART_CMD4 'B'

/************************************************************************/
/* VARIABLES GLOBALES Y ESTRUCTURAS DE DATOS                            */
/************************************************************************/
uint8_t uart_rx = '1';


// Estructura LCD (modo 4 bits)
LCD_4b lcd = {
	.rs = { &PORTD, &DDRD, PORTD2 },
	.e  = { &PORTD, &DDRD, PORTD3 },
	.d4 = { &PORTD, &DDRD, PORTD5 },
	.d5 = { &PORTD, &DDRD, PORTD6 },
	.d6 = { &PORTD, &DDRD, PORTD7 },
	.d7 = { &PORTB, &DDRB, PORTB0 }
};

/************************************************************************/
/* SETUP                                                                */
/************************************************************************/
void setup(void)
{
	// Inicializar UART
	UART_Init(UART_BAUD_9600_16MHZ, UART_INTERRUPTS_ENABLED);
	
	// Inicializar LCD
	LCD_Init_4b(&lcd);
	LCD_SetCursor(&lcd, 0, 0);
	LCD_WriteString(&lcd, "Ejecutando");
	sei();
}

int main(void)
{
	
	setup();
	while (1)
	{	
		// RECEPCIÓN DE COMANDOS UART
		switch(uart_rx)
		{
			case UART_CMD1:
			LCD_SetCursor(&lcd, 0, 1);
			LCD_WriteString(&lcd, "COMANDO 1");
			break;
			
			case UART_CMD2:
			LCD_SetCursor(&lcd, 0, 1);
			LCD_WriteString(&lcd, "COMANDO 2");
			break;
			
			default:
			break;
		}
		
		_delay_ms(200);
	}
}

/************************************************************************/
/* RUTINAS DE INTERRUPCIÓN                                              */
/************************************************************************/
ISR(USART_RX_vect)
{
	uart_rx = UDR0;
}