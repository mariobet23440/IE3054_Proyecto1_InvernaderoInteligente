/*
 * Esclavo_Sensor_DC.c
 *
 * Created: 15/02/2026 09:55:55 p. m.
 * Author : David Carranza
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

#include "ADC.h"
#include "I2C_Slave.h"
#include "dc_motor.h"
#include "UART.h"
#include "DHT11.h"
#include <stdio.h>

/************************************************************************/
/*                          CONFIGURACIÓN                               */
/************************************************************************/

#define SLAVE_ADDR		0x31
#define CMD_READ_SOIL	0x40
#define CMD_FAN_ON		0x41
#define CMD_FAN_OFF		0x42
#define CMD_FAN_PWM		0x43
#define CMD_READ_HUM    0x44
#define CMD_READ_TEMP   0x45


#define SOIL_POWER_PIN	PB0	 // D8

extern volatile uint8_t tx_data;


/************************************************************************/
/*                               MAIN                                    */
/************************************************************************/

// ... (tus includes y defines)

// Variables globales para guardar la última lectura
volatile uint8_t ultima_humedad = 0;
volatile uint8_t ultima_temperatura = 0;

int main(void)
{
	// ... inicializaciones ...
	uint16_t timer_sensor = 0;

	while(1)
	{
		// 1. LEER SENSOR CADA 2 SEGUNDOS (El DHT11 no soporta menos de 1 seg)
		if (timer_sensor++ > 2000) {
			DHT11_Read(&ultima_humedad, &ultima_temperatura);
			timer_sensor = 0;
		}

		// 2. ATENDER COMANDOS I2C (Ahora es instantáneo)
		uint8_t command = I2C_Slave_CheckCommand();
		
		if (command != 0xFF)
		{
			switch(command)
			{
				case CMD_READ_SOIL:
				PORTB |= (1 << SOIL_POWER_PIN);
				_delay_ms(5);
				tx_data = (uint8_t)(ADC_Read(0) >> 2);
				PORTB &= ~(1 << SOIL_POWER_PIN);
				break;
				
				case CMD_READ_HUM:
				tx_data = ultima_humedad; // Responde con lo que ya tiene
				break;

				case CMD_READ_TEMP:
				tx_data = ultima_temperatura; // Responde con lo que ya tiene
				break;
				
				// ... resto de casos (ventilador, etc)
			}
		}
		_delay_ms(1); // Pequeña demora para el contador
	}
}