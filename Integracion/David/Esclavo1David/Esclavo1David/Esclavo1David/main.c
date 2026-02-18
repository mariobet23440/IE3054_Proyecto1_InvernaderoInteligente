/*
 * Esclavo_Sensor_DC.c
 * Integración de DHT11 para Invernadero
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "ADC.h"
#include "I2C_Slave.h"
#include "dc_motor.h"
#include "UART.h"
#include "DHT11.h" // Librería del sensor

/************************************************************************/
/* CONFIGURACIÓN                                                       */
/************************************************************************/
#define SLAVE_ADDR		0x31
#define CMD_READ_SOIL	0x40
#define CMD_FAN_ON		0x41
#define CMD_FAN_OFF		0x42
#define CMD_FAN_PWM		0x43
#define CMD_READ_DHT	'T'   // Comando solicitado: 0x54

#define SOIL_POWER_PIN	PB0	 

extern volatile uint8_t tx_data;

// Variables globales para almacenar la última lectura
uint8_t humidity = 0;
uint8_t temperature = 0;

int	main(void)
{	
	uint8_t command; 
	
	// Inicializadores
	UART_Init(UART_BAUD_9600_16MHZ, UART_INTERRUPTS_DISABLED);
	UART_SendString("Esclavo Invernadero: Sistema Listo\r\n");
	
	I2C_Slave_Init(SLAVE_ADDR);
	ADC_Init();
	DC_Init();
	DHT11_Init(); // Inicializar DHT11
	
	// Configuración de pin de alimentación del sensor de suelo
	DDRB |= (1 << SOIL_POWER_PIN);
	PORTB &= ~(1 << SOIL_POWER_PIN);
	
	while(1)
	{
		command = I2C_Slave_CheckCommand();
		
		if (command != 0xFF)
		{
			switch(command)
			{
				case CMD_READ_SOIL:
				{
					PORTB |= (1 << SOIL_POWER_PIN);
					_delay_ms(10); // Breve estabilización
					uint16_t soil_value = ADC_Read(0);
					PORTB &= ~(1 << SOIL_POWER_PIN);
					
					tx_data = (uint8_t)(soil_value >> 2); // Conversión a 8 bits
					UART_SendString("Lectura Suelo enviada\r\n");
				}
				break;
				
				case CMD_READ_DHT:
				{
					// Intentar leer el sensor
					if (DHT11_Read(&humidity, &temperature) == 1) {
						// Para este ejemplo, cargamos la Temperatura en tx_data 
						// para que el Maestro la reciba de inmediato.
						tx_data = temperature; 
						UART_SendString("Lectura DHT11 Exitosa\r\n");
					} else {
						tx_data = 0xEE; // Código de error si falla la lectura
						UART_SendString("Error en DHT11\r\n");
					}
				}
				break;
				
				case CMD_FAN_ON:
					DC_On();
					break;
					
				case CMD_FAN_OFF:
					DC_Off();
					break;
				
				case CMD_FAN_PWM:
				{
					// Esperar el byte de velocidad que envía el maestro
					while((TWSR & 0xF8) != 0x80);
					uint8_t speed = TWDR;
					TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWINT);
					DC_SetSpeed(speed);
				}
				break;
			}		 
		}
	}
}