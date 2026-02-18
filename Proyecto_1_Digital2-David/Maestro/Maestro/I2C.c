/*
 * I2C.c
 *
 * Created: 12/02/2026 07:39:30 p. m.
 *  Author: Admin
 */ 

#include <avr/io.h>
#include <stdint.h>
#include "I2C.h"

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define I2C_TIMEOUT_LIMIT  10000


void I2C_MasterInit(unsigned long SCL_Clock, uint8_t Prescaler)
{
	DDRC &= ~((1 << DDC4) | (1 << DDC5));

	TWSR &= ~((1 << TWPS0) | (1 << TWPS1));

	switch(Prescaler)
	{
		case 1: break;
		case 4:  TWSR |= (1 << TWPS0); break;
		case 16: TWSR |= (1 << TWPS1); break;
		case 64: TWSR |= (1 << TWPS1) | (1 << TWPS0); break;
	}

	TWBR = ((F_CPU / SCL_Clock) - 16) / (2 * Prescaler);
}

uint8_t I2C_MasterStart()
{
	uint16_t timeout = 0;
	
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	
	while (!(TWCR & (1 << TWINT)))
	{
		if (timeout++ > I2C_TIMEOUT_LIMIT)
		return 0;  // timeout
	}

	uint8_t status = (TWSR & 0xF8);
	return (status == 0x08 || status == 0x10);
}

uint8_t I2C_MasterRepeatedStart(void)
{
	uint16_t timeout = 0;
	
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	
	while (!(TWCR & (1 << TWINT)))
	{
		if (timeout++ > I2C_TIMEOUT_LIMIT)
			return 0;
	}

	uint8_t status = (TWSR & 0xF8);
	return (status == 0x10);
}

void I2C_MasterStop(void)
{
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}

uint8_t I2C_Master_Write(uint8_t dato)
{
	uint16_t timeout = 0;
	
	TWDR = dato;
	TWCR = (1 << TWEN) | (1 << TWINT);
	
	while(!(TWCR & (1 << TWINT)))
	{
		if (timeout++ > I2C_TIMEOUT_LIMIT)
		return 0;  // timeout
	}

	return (TWSR & 0xF8);
}

uint8_t I2C_MasterRead(uint8_t *buffer, uint8_t ack)
{
	uint16_t timeout = 0;
	
	if(ack)
		TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWEA);
	else
		TWCR = (1 << TWEN) | (1 << TWINT);

	while(!(TWCR & (1 << TWINT)))
	{
		if (timeout++ > I2C_TIMEOUT_LIMIT)
			return 0;
	}

	uint8_t status = TWSR & 0xF8;

	if ((ack && status == 0x50) || (!ack && status == 0x58))
	{
		*buffer = TWDR;
		return 1;
	}

	return 0;
}
