/*
 * I2C.h
 *
 * Created: 12/02/2026 07:39:43 p. m.
 *  Author: Admin
 */ 


#ifndef I2C_H_
#define I2C_H_

#include <avr/io.h>
#include <stdint.h>

#define I2C_ACK  1
#define I2C_NACK 0

#define I2C_READ  1
#define I2C_WRITE 0

void I2C_MasterInit(unsigned long SCL_Clock, uint8_t Prescaler);
uint8_t I2C_MasterStart(void);
uint8_t I2C_MasterRepeatedStart(void);
void I2C_MasterStop(void);
uint8_t I2C_Master_Write(uint8_t dato);
uint8_t I2C_MasterRead(uint8_t *buffer, uint8_t ack);


#endif /* I2C_H_ */