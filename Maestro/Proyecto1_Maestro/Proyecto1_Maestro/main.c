/************************************************************************/
/* PROYECTO 1 - MAESTRO (LUZ + COMUNICACIÓN ESCLAVO 2)                  */
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

#define SLAVE2_ADDR 0x32

// TSL2561 (Luz)
#define TSL2561_ADDR        0x39
#define TSL2561_CMD_BIT     0x80
#define TSL2561_WORD_BIT    0x20
#define TSL2561_REG_CONTROL 0x00
#define TSL2561_REG_DATA0LO 0x0C
#define CMD_POWER_UP        (TSL2561_CMD_BIT | TSL2561_REG_CONTROL)
#define CMD_READ_DATA       (TSL2561_CMD_BIT | TSL2561_WORD_BIT | TSL2561_REG_DATA0LO)

/************************************************************************/
/* VARIABLES GLOBALES                                                   */
/************************************************************************/
volatile uint8_t uart_rx = 0;

LCD_4b lcd = {
	.rs = { &PORTD, &DDRD, PORTD2 },
	.e  = { &PORTD, &DDRD, PORTD3 },
	.d4 = { &PORTD, &DDRD, PORTD5 },
	.d5 = { &PORTD, &DDRD, PORTD6 },
	.d6 = { &PORTD, &DDRD, PORTD7 },
	.d7 = { &PORTB, &DDRB, PORTB0 }
};

/************************************************************************/
/* PROTOTIPOS                                                           */
/************************************************************************/
void I2CCommandExecution(void);
void UARTCommandProcessing(void);
void TSL2561_Init_Sensor(void);
uint16_t TSL2561_Read_Luminosity(void);

/************************************************************************/
/* SETUP Y MAIN                                                         */
/************************************************************************/
void setup(void) {
	UART_Init(UART_BAUD_9600_16MHZ, UART_INTERRUPTS_ENABLED);
	I2C_MasterInit(100000, 1);
	
	LCD_Init_4b(&lcd);
	LCD_Clear(&lcd);
	LCD_WriteString(&lcd, "Luz + Esclavo 2");

	// PB4 para Esclavo 2, PB5 para Sensor de Luz
	DDRB |= (1 << DDB4) | (1 << DDB5);
	PORTB &= ~((1 << PORTB4) | (1 << PORTB5));

	TSL2561_Init_Sensor();
	sei();
	_delay_ms(1000);
	LCD_Clear(&lcd);
}

int main(void) {
	setup();

	while (1) {
		UARTCommandProcessing();
		I2CCommandExecution();
		_delay_ms(200);
	}
}

/************************************************************************/
/* LÓGICA DE SENSORES Y COMUNICACIÓN                                    */
/************************************************************************/
void I2CCommandExecution(void) {
	char buffer[16];

	// --- 1. LEER LUZ (TSL2561) ---
	PORTB |= (1 << PORTB5); // LED indicador Luz
	uint16_t luz = TSL2561_Read_Luminosity();
	sprintf(buffer, "Luz: %5u lux", luz);
	LCD_SetCursor(&lcd, 0, 0);
	LCD_WriteString(&lcd, buffer);
	PORTB &= ~(1 << PORTB5);

	_delay_ms(10); // Pausa mínima entre dispositivos

	// --- 2. ENVIAR 'A' Y 'B' AL ESCLAVO 2 ---
	PORTB |= (1 << PORTB4); // LED indicador Esclavo 2
	if (I2C_MasterStart()) {
		// Dirección de escritura: (0x32 << 1) | 0
		if (I2C_Master_Write((SLAVE2_ADDR << 1) | I2C_WRITE) == 0x18) {
			I2C_Master_Write('A');
			_delay_ms(5); // Pequeño margen para el esclavo
			I2C_Master_Write('B');
			
			LCD_SetCursor(&lcd, 0, 1);
			LCD_WriteString(&lcd, "S2: Env OK     ");
			} else {
			LCD_SetCursor(&lcd, 0, 1);
			LCD_WriteString(&lcd, "S2: No Resp.   ");
		}
		I2C_MasterStop();
	}
	PORTB &= ~(1 << PORTB4);
}

void UARTCommandProcessing(void) {
	if (uart_rx == 0) return;
	
	// Mostramos brevemente el comando UART en una esquina
	LCD_SetCursor(&lcd, 12, 1);
	switch(uart_rx) {
		case UART_CMD1: LCD_WriteString(&lcd, "[A]"); break;
		case UART_CMD2: LCD_WriteString(&lcd, "[B]"); break;
		case UART_CMD3: LCD_WriteString(&lcd, "[C]"); break;
		case UART_CMD4: LCD_WriteString(&lcd, "[D]"); break;
	}
}

/************************************************************************/
/* IMPLEMENTACIÓN I2C                                                   */
/************************************************************************/

void TSL2561_Init_Sensor(void) {
	if (I2C_MasterStart()) {
		// Intentamos escribir a la dirección. I2C_Master_Write devuelve 0x18 si hay ACK.
		if (I2C_Master_Write((TSL2561_ADDR << 1) | I2C_WRITE) == 0x18) {
			I2C_Master_Write(CMD_POWER_UP);
			I2C_Master_Write(0x03); // Encender
			LCD_SetCursor(&lcd, 0, 1);
			LCD_WriteString(&lcd, "Luz: Conectado ");
			} else {
			LCD_SetCursor(&lcd, 0, 1);
			LCD_WriteString(&lcd, "Luz: No Detect.");
		}
		I2C_MasterStop();
	}
}

uint16_t TSL2561_Read_Luminosity(void) {
	uint8_t lo = 0, hi = 0;
	if (I2C_MasterStart()) {
		I2C_Master_Write((TSL2561_ADDR << 1) | I2C_WRITE);
		I2C_Master_Write(CMD_READ_DATA);
		I2C_MasterRepeatedStart();
		I2C_Master_Write((TSL2561_ADDR << 1) | I2C_READ);
		I2C_MasterRead(&lo, 1);
		I2C_MasterRead(&hi, 0);
		I2C_MasterStop();
	}
	return (uint16_t)((hi << 8) | lo);
}

ISR(USART_RX_vect) {
	uart_rx = UDR0;
}