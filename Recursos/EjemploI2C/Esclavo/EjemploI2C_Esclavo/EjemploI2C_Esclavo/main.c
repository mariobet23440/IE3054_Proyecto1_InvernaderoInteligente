#define F_CPU 16000000UL
#include <avr/io.h>
#include "I2C.h"

#define MY_ADDR 0x30

int main(void) {
	// Configurar LED (Pin 13 de Arduino / PB5) como salida
	DDRD |= (1 << DDD5);
	
	// Inicializar esclavo en dirección 0x30
	I2C_Slave_Init(MY_ADDR);
	
	uint8_t status;

	while(1) {
		// Esperar a que el hardware TWI ponga la bandera TWINT en alto
		while (!(TWCR & (1 << TWINT)));

		// Leer el estado del bus (mascará de los bits de estado)
		status = (TWSR & 0xF8);

		switch(status) {
			case 0x60: // Me llamaron por mi dirección (SLA+W recibido)
			case 0x70: // Llamada general (opcional)
			// Limpiar bandera y habilitar ACK para recibir el dato
			TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
			break;

			case 0x80: // Dato recibido correctamente
			if (TWDR == 'A') {
				PORTD |= (1 << PORTD5);  // Encender LED
				} else if (TWDR == 'B') {
				PORTD &= ~(1 << PORTD5); // Apagar LED
			}
			// Limpiar bandera para seguir operando
			TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
			break;

			case 0xA0: // Se recibió un STOP o un START repetido
			// Reiniciar y esperar nueva comunicación
			TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
			break;

			default:
			// Si algo falla, reiniciar el bus
			TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
			break;
		}
	}
}