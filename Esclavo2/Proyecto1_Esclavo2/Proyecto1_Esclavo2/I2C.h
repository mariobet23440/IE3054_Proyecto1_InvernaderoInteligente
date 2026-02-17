/*
 * I2C.h
 *
 * Created: 5/02/2026 19:42:09
 * Author: Mario Alejandro Betancourt Franco
 *
 * Descripción: Librería para comunicación TWI/I2C en ATmega328P
 */ 

#ifndef I2C_H_
#define I2C_H_

#include <avr/io.h>
#include <stdint.h>

// --- Macros útiles para hacer el código legible ---

// Para usar con I2C_MasterRead
#define I2C_ACK  1  // Responder con ACK (pedir otro byte)
#define I2C_NACK 0  // Responder con NACK (último byte, finalizar)

// Para usar al enviar la dirección (address << 1 | BIT)
#define I2C_READ  1
#define I2C_WRITE 0

// --- Prototipos de funciones (Maestro) ---

// Inicializa el hardware I2C
// SCL_Clock: Frecuencia deseada (ej. 100000UL para 100kHz)
// Prescaler: 1, 4, 16 o 64
void I2C_MasterInit(unsigned long SCL_Clock, uint8_t Prescaler);

// Genera condición de Start
// Retorna: 1 si éxito, 0 si falla
uint8_t I2C_MasterStart(void);

// Genera condición de Repeated Start (para no soltar el bus)
// Retorna: 1 si éxito, 0 si falla
uint8_t I2C_MasterRepeatedStart(void);

// Genera condición de Stop (Libera el bus)
void I2C_MasterStop(void);

// Envía un byte al bus
// dato: Byte a enviar
// Retorna: Estado del registro TWSR (útil para ver si hubo ACK)
uint8_t I2C_Master_Write(uint8_t dato);

// Lee un byte del bus
// *buffer: Puntero a la variable donde guardar el dato
// ack: I2C_ACK (1) para pedir más, I2C_NACK (0) para terminar
// Retorna: 1 si leyó correctamente, 0 si hubo error
uint8_t I2C_MasterRead(uint8_t *buffer, uint8_t ack);


// --- Prototipos de funciones (Esclavo) ---

// Inicializa el ATmega como esclavo en la dirección indicada
void I2C_Slave_Init(uint8_t address);


#endif /* I2C_H_ */