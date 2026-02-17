/************************************************************************
PROYECTO 1 - ESCLAVO 2
Creado por: Mario Alejandro Betancourt Franco (23440)
Descripción: 
    - Controla un LED/Motor basado en comandos del Maestro ('A'/'B').
    - Envía datos de "humedad" al Maestro cuando este los solicita.
************************************************************************/

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h> // Librería necesaria para rand()
#include "I2C.h"

/************************************************************************/
/* DEFINICIONES                                                         */
/************************************************************************/
#define SLAVE2_ADDR 0x32

/************************************************************************/
/* VARIABLES GLOBALES                                                   */
/************************************************************************/
volatile uint8_t i2c_rx = 0;      // Dato recibido del maestro
volatile uint8_t humedad_val = 0; // Valor a enviar al maestro

/************************************************************************/
/* CONFIGURACIÓN                                                        */
/************************************************************************/
void setup(void)
{
    // Configurar PB5 (LED/Motor) como salida
    DDRB |= (1 << DDB5);
    
    // Inicializar I2C Esclavo
    I2C_Slave_Init(SLAVE2_ADDR);
    
    // Habilitar TWI con Interrupciones y ACKs
    // TWEN: Habilitar, TWEA: ACK, TWIE: Interrupción, TWINT: Limpiar bandera
    TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWIE) | (1 << TWINT);
    
    sei(); // Habilitar interrupciones globales
}

/************************************************************************/
/* PROGRAMA PRINCIPAL                                                   */
/************************************************************************/
int main(void) 
{
    setup();

    while(1) 
    {
        // 1. Lógica de Actuadores (Comandos del Maestro)
        if (i2c_rx == 'A') {
            PORTB |= (1 << PORTB5);  // Encender
        } else if (i2c_rx == 'B') {
            PORTB &= ~(1 << PORTB5); // Apagar
        }

        // 2. Simulación de lectura de sensor
        // En un caso real, aquí iría la lectura del ADC
        // humedad_val = leer_ADC(canal_humedad);
        humedad_val = rand() % 256;
        
        _delay_ms(10); // Pequeña pausa de estabilidad
    }
}

/************************************************************************/
/* VECTOR DE INTERRUPCIÓN I2C (TWI)                                     */
/************************************************************************/
ISR(TWI_vect) 
{
    uint8_t status = (TWSR & 0xF8); // Leer el registro de estado

    switch(status) 
    {
        // --- CASO 1: MAESTRO ESCRIBE (RECIBIR DATOS) ---
        case 0x60: // Dirección de esclavo + W recibida
        case 0x70: // Llamada general recibida
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
            break;

        case 0x80: // Dato recibido correctamente
            i2c_rx = TWDR; // Guardar comando ('A', 'B', etc.)
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
            break;

        // --- CASO 2: MAESTRO LEE (ENVIAR DATOS) ---
        case 0xA8: // Dirección de esclavo + R recibida (nos piden datos)
        case 0xB8: // Dato enviado y recibimos un ACK (el maestro quiere más)
            TWDR = humedad_val; // Cargar el valor del sensor al bus
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
            break;

        case 0xC0: // Dato enviado y recibimos un NACK (el maestro ya no quiere más)
        case 0xA0: // Se recibió un STOP o un Repeated START
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
            break;

        // --- CASO POR DEFECTO: ERRORES O ESTADOS NO MANEJADOS ---
        default:
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
            break;
    }
}