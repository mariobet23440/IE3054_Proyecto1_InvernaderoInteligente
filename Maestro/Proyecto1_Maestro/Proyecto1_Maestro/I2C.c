/************************************************************************/
/*	LIBRERÍA I2C (Ver 2.1) - CON TIMEOUT
	ACTUALIZADA POR: Mario Alejandro Betancourt Franco
	DESCRIPCIÓN: Se agregaron contadores de seguridad en los bucles while
*************************************************************************/
#include "I2C.h"

#ifndef F_CPU
#define F_CPU 16000000UL 
#endif

// Definición de Timeout (ajustable según necesidad)
#define I2C_TIMEOUT 60000

// --- FUNCIONES INTERNAS DE AYUDA ---

// Esta función espera a que la bandera TWINT se levante con un límite de tiempo
static uint8_t I2C_Wait_With_Timeout(void) {
    uint16_t timer = I2C_TIMEOUT;
    while (!(TWCR & (1 << TWINT)) && timer > 0) {
        timer--;
    }
    return (timer > 0); // Retorna 1 si tuvo éxito, 0 si hubo timeout
}

// --- FUNCIONES DE LIBRERÍA ---

void I2C_MasterInit(unsigned long SCL_Clock, uint8_t Prescaler)
{
    DDRC &= ~((1 << DDC4) | (1 << DDC5));
    TWSR &= ~((1 << TWPS0) | (1 << TWPS1)); 
    
    switch(Prescaler)
    {
        case 1:  break; 
        case 4:  TWSR |= (1 << TWPS0); break; 
        case 16: TWSR |= (1 << TWPS1); break; 
        case 64: TWSR |= (1 << TWPS1) | (1 << TWPS0); break; 
    }
    TWBR = ((F_CPU / SCL_Clock) - 16) / (2 * Prescaler);
}

uint8_t I2C_MasterStart()
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    
    if (!I2C_Wait_With_Timeout()) return 0; // Error por timeout
    
    return ((TWSR & 0xF8) == 0x08); 
}

uint8_t I2C_MasterRepeatedStart(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    
    if (!I2C_Wait_With_Timeout()) return 0;
    
    return ((TWSR & 0xF8) == 0x10);
}

void I2C_MasterStop(void)
{
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    // Nota: El hardware libera el bus automáticamente, no requiere wait.
}

uint8_t I2C_Master_Write(uint8_t dato)
{   
    TWDR = dato;
    TWCR = (1 << TWEN) | (1 << TWINT);
    
    if (!I2C_Wait_With_Timeout()) return 0xFF; // Retornar estado de error
    
    return (TWSR & 0xF8);
}

uint8_t I2C_MasterRead(uint8_t *buffer, uint8_t ack)
{
    if(ack) 
        TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWEA); 
    else    
        TWCR = (1 << TWEN) | (1 << TWINT);               
    
    if (!I2C_Wait_With_Timeout()) return 0; 
    
    uint8_t status = TWSR & 0xF8;
    
    if ( (ack && status == 0x50) || (!ack && status == 0x58) ) {
        *buffer = TWDR; 
        return 1; 
    }
    
    return 0; 
}

void I2C_Slave_Init(uint8_t address)
{
    TWAR = (address << 1); 
    TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT);
}