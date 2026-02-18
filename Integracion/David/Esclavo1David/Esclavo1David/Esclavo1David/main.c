/*
 * Esclavo_Sensor_DC.c
 *
 * Created: 15/02/2026 09:55:55 p. m.
 * Author : David Carranza
 * Adición de librería DHT11 : Mario Betancourt
 */ 

#define F_CPU 16000000UL

/************************************************************************/
/* LIBRERÍAS                                                            */
/************************************************************************/

#include <avr/io.h>
#include <util/delay.h>

#include "ADC.h"
#include "I2C_Slave.h"
#include "dc_motor.h"
#include "UART.h"
#include <stdio.h>

/************************************************************************/
/*  CONFIGURACIÓN						                                */
/************************************************************************/
#define SLAVE_ADDR		0x31
#define CMD_READ_SOIL	0x40
#define CMD_FAN_ON		0x41
#define CMD_FAN_OFF		0x42
#define CMD_FAN_PWM		0x43
#define CMD_READ_TEMP	0x44

// Pin de
#define SOIL_POWER_PIN	PB0	 // D8

extern volatile uint8_t tx_data;


/************************************************************************/
/*                               MAIN                                    */
/************************************************************************/

int	main(void)
{	
	//uint16_t soil_value;
	uint8_t command; 
	
	// Inicializadores
	UART_Init(UART_BAUD_9600_16MHZ, UART_INTERRUPTS_DISABLED);
	UART_SendString("Sensor listo\r\n");
	I2C_Slave_Init(SLAVE_ADDR);
	ADC_Init();
	DC_Init();
	
	// Configuración de pin de alimentación del sensor
	DDRB |= (1 << SOIL_POWER_PIN);
	PORTB &= ~(1 << SOIL_POWER_PIN);	// Apagado inicialmente
	
	while(1)
	{
		command = I2C_Slave_CheckCommand();
		
		if (command != 0xFF)
		
		{
			switch(command)
			{
				case CMD_READ_SOIL:
				{
					 uint16_t soil_value;
					
					 // Encender sensor
					 PORTB |= (1 << SOIL_POWER_PIN);
					 //_delay_ms(50);		// Estabilizar lectura
			 
					 soil_value = ADC_Read(0);	// A0
					 
					 // Apagar sensor
					 PORTB &= ~(1 << SOIL_POWER_PIN);
			 
					 // Convertir a 8 bits para I2C
					 uint8_t soil_8bit = soil_value >> 2;
			 
					 // Preparar respuesta
					 tx_data = soil_8bit;
					 
				}
				break;
				
				case CMD_FAN_ON:
					DC_On();
					break;
					
				case CMD_FAN_OFF:
					DC_Off();
					break;
				
				case CMD_FAN_PWM:
				// El esclavo debe esperar el siguiente byte que contiene la velocidad
				// Función para leer el dato que viene después del comando
				{
					// Esperar siguiente dato
					while((TWSR & 0xF8) != 0x80);
					uint8_t speed = TWDR;

					TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWINT);

					DC_SetSpeed(speed);
				}
				break;
				
				case CMD_READ_TEMP:
				{
					
				}
				break;
			}		 
		}
	}
}
