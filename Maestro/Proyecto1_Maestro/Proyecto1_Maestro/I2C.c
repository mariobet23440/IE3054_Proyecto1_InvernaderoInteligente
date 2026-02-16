/************************************************************************/
/*	LIBRERÍA I2C (Ver 2.0) - C FILE
	CREADA POR: Mario Alejandro Betancourt Franco (23440)
				(Basada en la librería de Pablo Mazariegos)
	DESCRIPCIÓN: Librería para comunicación I2C.
	FUNCIONES DISPONIBLES:
		- Inicialización de maestros y esclavos
		- Generación de condiciones START, STOP y REPEATED_START
		- Transmisión de datos desde maestro (Master Transmitter)
		- Recepción de datos de maestro (Master Receiver)
		
	ÚLTIMA ACTUALIZACIÓN: 12/02/2026						            */
/************************************************************************/
#include "I2C.h"

// Definir F_CPU si no está definido en el proyecto
#ifndef F_CPU
#define F_CPU 16000000UL 
#endif

// Inicialización de maestro
void I2C_MasterInit(unsigned long SCL_Clock, uint8_t Prescaler)
{
    // Poner PC4 (SDA) y PC5 (SCL) como entradas (pull-ups externos recomendados)
    DDRC &= ~((1 << DDC4) | (1 << DDC5));
    
    // Limpiar bits de prescaler antes de configurar
    TWSR &= ~((1 << TWPS0) | (1 << TWPS1)); 
    
    // Colocar prescaler en TWSR
    switch(Prescaler)
    {
        case 1:  break; // 00
        case 4:  TWSR |= (1 << TWPS0); break; // 01
        case 16: TWSR |= (1 << TWPS1); break; // 10
        case 64: TWSR |= (1 << TWPS1) | (1 << TWPS0); break; // 11
    }
    
    // Cálculo en registro de velocidad (Bit Rate Register)
    // Formula: SCL = F_CPU / (16 + 2*TWBR * Prescaler)
    TWBR = ((F_CPU / SCL_Clock) - 16) / (2 * Prescaler);
}

// Transmitir condición START
uint8_t I2C_MasterStart()
{
    // CORRECCIÓN: Se cambió el '1' por '|'
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    
    // Esperar hasta que la bandera TWINT se habilite (operación completa)
    while (!(TWCR & (1 << TWINT)));
    
    // Verificar estado: 0x08 = START transmitido
    uint8_t status = (TWSR & 0xF8);
    
    // Retorna 1 si fue exitoso (0x08), 0 si falló
    return (status == 0x08);    
}

// Transmitir condición Repeated START
uint8_t I2C_MasterRepeatedStart(void)
{
    // CORRECCIÓN: Se cambió el '1' por '|'
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    
	// Esperar hasta que se levante la bandera TWINT
    while (!(TWCR & (1 << TWINT)));
    
    // Verificar estado: 0x10 = Repeated START transmitido
    uint8_t status = (TWSR & 0xF8);
    
	// Retornar variable booleana para indicar que se transmitió el START
    return (status == 0x10);
}

// Generar condición STOP (Faltaba esta función)
void I2C_MasterStop(void)
{
    // Enviar STOP condition
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    
    // NOTA: Para el STOP no se espera a TWINT, el hardware lo hace automáticamente
}


// Enviar dato desde maestro (Master Transmitter)
// Esto puede ser la dirección del SLA + R/W
// o un bit de datos
uint8_t I2C_Master_Write(uint8_t dato)
{   
    TWDR = dato;
    TWCR = (1 << TWEN) | (1 << TWINT);
    
    while(!(TWCR & (1 << TWINT)));    
    
    uint8_t status = TWSR & 0xF8;
    
    // Retornar estado para verificar ACKs (0x18, 0x28, etc)
    return status;
}

// Recibir dato en maestro (Master Receiver)
// buffer: puntero donde se guardará el dato leído
// ack: 1 para enviar ACK (pedir más datos), 0 para enviar NACK (último dato)
uint8_t I2C_MasterRead(uint8_t *buffer, uint8_t ack)
{
    if(ack) 
        TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWEA); // Responder con ACK
    else    
        TWCR = (1 << TWEN) | (1 << TWINT);               // Responder con NACK
    
    // Esperar a que llegue el dato
    while(!(TWCR & (1 << TWINT))); 
    
    // Leer el estado
    uint8_t status = TWSR & 0xF8;
    
    // Verificar si la lectura fue exitosa según lo esperado
    // 0x50: Dato recibido, ACK retornado
    // 0x58: Dato recibido, NACK retornado
    if ( (ack && status == 0x50) || (!ack && status == 0x58) ) {
        *buffer = TWDR; // Guardar el dato recibido en el puntero
        return 1; // Éxito
    }
    
    return 0; // Error en la recepción
}

// Inicialización de Esclavo
void I2C_Slave_Init(uint8_t address)
{
    // Configurar pines como entrada (opcional, el hardware TWI lo maneja)
    DDRC &= ~((1 << DDC4) | (1 << DDC5));
    
    // Cargar dirección del esclavo
    // La dirección va en los 7 bits más significativos.
    // El bit 0 (TWGCE) es para General Call (0 aquí para desactivarlo)
    TWAR = (address << 1); 
    
    // Habilitar TWI:
    // TWEN: Habilita el módulo
    // TWEA: Habilita el envío de ACKs (esencial para ser esclavo)
    // TWINT: Limpia la bandera para empezar
    TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT);
}