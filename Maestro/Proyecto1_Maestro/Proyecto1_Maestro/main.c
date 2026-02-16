/************************************************************************/
/*	PROYECTO 1 - MAESTRO
	CREADA POR: Mario Alejandro Betancourt Franco (23440)
	DESCRIPCIÓN: Programa para maestro de invernadero inteligente
	RESUMEN DE FUNCIONAMIENTO DEL DISPOSITIVO:
		- Se comunica con cada esclavo al que está conectado
		- Mantiene comunicación con un ESP32 vía UART
		- Controla la pantalla LCD.
		
	ÚLTIMA ACTUALIZACIÓN: 12/02/2026						            */
/************************************************************************/

#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "HD44780_4b.h"
#include "UART.h"
#include "I2C.h"

/************************************************************************/
/* DEFINICIONES                                                         */
/************************************************************************/
#define UART_CMD1 'A'
#define UART_CMD2 'B'
#define UART_CMD3 'C'
#define UART_CMD4 'D'

#define SLAVE_ADDR 0x30

/************************************************************************/
/* VARIABLES GLOBALES Y ESTRUCTURAS DE DATOS                            */
/************************************************************************/
uint8_t uart_rx = '1';
double time_delay = 500;


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
	
	// Inicializar I2C a 100kHz con prescaler de 1
	I2C_MasterInit(100000, 1);
	
	sei();
}

int main(void)
{
	
	setup();
	while (1)
	{	
		// COMUNICACIÓN I2C
		// Iniciar comunicación
		if (I2C_MasterStart()) {
			// Comunicarse con el esclavo y recibir datos
			if (I2C_Master_Write((SLAVE_ADDR << 1) | I2C_WRITE) == 0x18) {
				I2C_Master_Write('A');
				_delay_ms(500);
				
				// Mandar 'B' para que el esclavo apague el LED
				I2C_Master_Write('B');
			}
			
			// 4. Siempre cerrar con STOP
			I2C_MasterStop();
		}
		
		// 1. Recepción de comando UART
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
			
			case UART_CMD3:
			LCD_SetCursor(&lcd, 0, 1);
			LCD_WriteString(&lcd, "COMANDO 3");
			break;
			
			case UART_CMD4:
			LCD_SetCursor(&lcd, 0, 1);
			LCD_WriteString(&lcd, "COMANDO 4");
			break;
			
			default:
			break;
		}
		
		_delay_ms(250);
		
		
	}
}

/************************************************************************/
/* RUTINAS DE INTERRUPCIÓN                                              */
/************************************************************************/
ISR(USART_RX_vect)
{
	uart_rx = UDR0;
	PORTB ^= (1 << PORTB5); // TIP: Haz que el LED L parpadee al recibir datos
}