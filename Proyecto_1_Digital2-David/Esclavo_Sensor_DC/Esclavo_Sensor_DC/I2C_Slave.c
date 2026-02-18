/*
 * I2C.c
 *
 * Created: 05/02/2026 08:12:55 p. m.
 *  Author: David Carranza
 */ 

#include "I2C_Slave.h"
volatile uint8_t tx_data = 0;   // dato a transmitir

void I2C_Slave_Init(uint8_t address)
{
	TWAR = (address << 1);     // Dirección esclavo
	TWCR = (1 << TWEN) |       // Habilitar TWI
	(1 << TWEA) |       // Habilitar ACK
	(1 << TWINT);       // Limpiar bandera
}

uint8_t I2C_Slave_CheckCommand(void)
{
	if (!(TWCR & (1 << TWINT)))
	return 0xFF;

	uint8_t status = TWSR & 0xF8;

	switch(status)
	{
		// ==========================
		// MASTER WRITE (SLA+W)
		// ==========================

		case 0x60:  // SLA+W recibido
		TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWINT);
		break;

		case 0x80:  // Dato recibido
		{
			uint8_t cmd = TWDR;
			TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWINT);
			return cmd;
		}

		case 0xA0:  // STOP
		TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWINT);
		break;

		// ==========================
		// MASTER READ (SLA+R)
		// ==========================

		case 0xA8:  // SLA+R recibido
		TWDR = tx_data;  // cargar dato
		TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWINT);
		break;

		case 0xB8:  // dato transmitido, ACK recibido
		TWDR = tx_data;
		TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWINT);
		break;

		case 0xC0:  // NACK recibido
		TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWINT);
		break;

		default:
		TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWINT);
		break;
	}

	return 0xFF;
}