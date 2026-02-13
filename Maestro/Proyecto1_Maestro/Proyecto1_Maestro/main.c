/*
 * Proyecto1_Maestro.c
 *
 * Created: 10/02/2026 14:57:37
 * Author : mario
 */ 

#include <avr/io.h>
#include "HD44780_4b.h"
#include "UART.h"

/************************************************************************/
/* ESTRUCTURAS DE DATOS                                                 */
/************************************************************************/
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
	
	// Inicializar LCD
	LCD_Init_4b(&lcd);
	LCD_SetCursor(&lcd, 0, 0);
	LCD_WriteString(&lcd, "Hola Mario");

	LCD_SetCursor(&lcd, 0, 1);
	LCD_WriteString(&lcd, "HD44780 4b");

}

int main(void)
{
	

	
	while (1)
	{
		// loop infinito
	}
}