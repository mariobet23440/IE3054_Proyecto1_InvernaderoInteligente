/************************************************************************/
/*	LIBRERÍA I2C (Ver 2.1) - HEADER FILE
	CREADA POR: Mario Alejandro Betancourt Franco (23440)
	DESCRIPCIÓN: Definiciones y prototipos para comunicación I2C
*************************************************************************/

#ifndef I2C_H_
#define I2C_H_

#include <avr/io.h>

// Definiciones útiles para claridad en el código
#define I2C_WRITE 0
#define I2C_READ  1

/************************************************************************/
/* PROTOTIPOS DE FUNCIONES                                              */
/************************************************************************/

// Inicialización del Maestro
void I2C_MasterInit(unsigned long SCL_Clock, uint8_t Prescaler);

// Condiciones de Inicio y Parada
uint8_t I2C_MasterStart(void);
uint8_t I2C_MasterRepeatedStart(void);
void I2C_MasterStop(void);

// Escritura y Lectura del Maestro
uint8_t I2C_Master_Write(uint8_t dato);
uint8_t I2C_MasterRead(uint8_t *buffer, uint8_t ack);

// Inicialización del Esclavo
void I2C_Slave_Init(uint8_t address);

#endif /* I2C_H_ */