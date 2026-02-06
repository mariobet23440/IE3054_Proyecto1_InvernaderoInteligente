#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "I2C.h"

#define SLAVE_ADDR 0x30

int main(void) {
	// Inicializar I2C a 100kHz con prescaler de 1
	I2C_MasterInit(100000, 1);
	
	while(1) {
		// 1. Intentar iniciar comunicación
		if (I2C_MasterStart()) {
			
			// 2. Dirección + Write (0)
			// El estado 0x18 significa que el esclavo recibió su dirección y dio ACK
			if (I2C_Master_Write((SLAVE_ADDR << 1) | I2C_WRITE) == 0x18) {
				
				// 3. Mandar el dato (un caracter 'A' para prender, 'B' para apagar)
				I2C_Master_Write('A');
				_delay_ms(500);
				
				// Mandar 'B' para que el esclavo apague el LED
				I2C_Master_Write('B');
			}
			
			// 4. Siempre cerrar con STOP
			I2C_MasterStop();
		}
		
		_delay_ms(500);
	}
}