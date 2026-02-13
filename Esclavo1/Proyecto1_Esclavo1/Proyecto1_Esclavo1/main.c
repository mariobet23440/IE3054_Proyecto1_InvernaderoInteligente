/************************************************************************
PROYECTO 1 - ESCLAVO 1
Creado por: Mario Alejandro Betancourt Franco (23440)
Descripción: Esclavo recibe comandos de maestro y controla motor DC
			 Recibe datos de sensor de humedad y los envía al maestro
************************************************************************/

/************************************************************************/
/* LIBRERÍAS	                                                        */
/************************************************************************/
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include "I2C.h"

/************************************************************************/
/* DEFINICIONES	                                                        */
/************************************************************************/
#define SLAVE1_ADDR 0x30

/************************************************************************/
/* VARIABLES GLOBALES                                                   */
/************************************************************************/
// Variable volátil para guardar el dato recibido (se usa volátil porque cambia en la ISR)
volatile uint8_t i2c_rx = 0;

/************************************************************************/
/* SETUP                                                                */
/************************************************************************/
void setup(void)
{
	// Configurar LED (PB5) como salida
	DDRB |= (1 << DDB5);
	
	// 1. Inicializar hardware esclavo
	I2C_Slave_Init(SLAVE1_ADDR);
	
	// 2. Habilitar la interrupción del TWI y ACKs
	// TWEN: Habilitar modulo, TWEA: Enviar ACKs, TWIE: Interrupción habilitada
	TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWIE) | (1 << TWINT);
	
	// 3. Habilitar interrupciones globales
	sei();
}


/************************************************************************/
/* MAINLOOP                                                             */
/************************************************************************/
int main(void) {
	
	setup();

	while(1) {
		if (i2c_rx == 'A') {
			PORTB |= (1 << PORTB5);
			} else if (i2c_rx == 'B') {
			PORTB &= ~(1 << PORTB5);
		}
	}
}

// --- VECTOR DE INTERRUPCIÓN DEL TWI ---
ISR(TWI_vect) {
	uint8_t status = (TWSR & 0xF8); // Leer estado

	switch(status) {
		case 0x60: // SLA+W recibido (nos hablaron)
		case 0x70: // Llamada general
		// Limpiar TWINT y mantener ACKs e Interrupciones habilitadas
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
		break;

		case 0x80: // Dato recibido
		i2c_rx = TWDR; // Guardamos lo que mandó el maestro
		// Volver a habilitar para el siguiente byte
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
		break;

		case 0xA0: // Recibimos STOP o Repeated START
		// Reiniciar para la próxima transmisión
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
		break;

		default:
		// En caso de error, resetear bandera y seguir escuchando
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
		break;
	}
}