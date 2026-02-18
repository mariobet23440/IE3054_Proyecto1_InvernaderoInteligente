/*
 * Proyecto1_Esclavo_SERVO_DC.c
 *
 * Created: 05/02/2026 07:22:32 p. m.
 * Author : Mario Bentacourt y David Carranza
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "I2C_Slave.h"
#include "servo_timer1.h"
#include "stepper_uln2003.h"

/************************************************************************/
/* DIRECCIÓN I2C                                                        */
/************************************************************************/
#define SLAVE_ADDR 0x30

/************************************************************************/
/* COMANDOS                                                             */
/************************************************************************/
#define CMD_PUMP_START	0x20
#define CMD_PUMP_STOP	0x21
#define CMD_SERVO_OPEN	0x10
#define CMD_SERVO_CLOSE	0x11

/************************************************************************/
/* MAIN                                                                 */
/************************************************************************/

int main(void)
{
	I2C_Slave_Init(SLAVE_ADDR);
	Servo_Init();
	Stepper_Init();

	uint8_t cmd;

	while(1)
	{
		cmd = I2C_Slave_CheckCommand();
		
		if(cmd != 0xFF)
		{

			switch(cmd)
			{
				case CMD_PUMP_START:
					Pump_Start();
					break;

				case CMD_PUMP_STOP:
					Pump_Stop();
					break;

				case CMD_SERVO_OPEN:
					Servo_SetAngle(120);
					break;

				case CMD_SERVO_CLOSE:
					Servo_SetAngle(20);
					break;
			}

			
		}
		
		// IMPORTANTE
		Stepper_Task();
	}
}

	
/*
// COMPROBAR FUNCIONAMIENTO

int main(void)
{
	Servo_Init();

	while (1)
	{
		Servo_SetAngle(0);
		_delay_ms(2000);
		Servo_SetAngle(180);
		_delay_ms(2000);
	}
}
*/

/*
int main(void)
{
	// Inicializaciones
	Servo_Init();
	TWI_Slave_Init(SLAVE_SERVO_DC_ADDR);

	while (1) {
		// Esperar evento I2C
		if ((TWSR & 0xF8) == 0x80) // Dato recibido
		{
			command = TWDR;

			switch (command)
			{
				case CMD_SERVO_SET_ANGLE:
				angle = TWI_Slave_Read();
				Servo_SetAngle(angle);
				break;

				case CMD_SERVO_OPEN:
				Servo_SetAngle(SERVO_OPEN_ANGLE);
				break;

				case CMD_SERVO_CLOSE:
				Servo_SetAngle(SERVO_CLOSE_ANGLE);
				break;

				default:
				// comando no válido
				break;
			}

			// Preparar siguiente recepción
			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
		}
	}
}
*/