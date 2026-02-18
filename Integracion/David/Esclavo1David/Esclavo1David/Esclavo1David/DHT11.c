#include "dht11.h"

// Función auxiliar interna (static para que no sea visible fuera de este archivo)
static uint8_t wait_for_state(uint8_t state) {
	uint8_t timeout = 0;
	while (((DHT_PIN_REG & (1 << DHT_BIT)) ? 1 : 0) != state) {
		_delay_us(1);
		timeout++;
		if (timeout > 200) return 0;
	}
	return 1;
}

void DHT11_Init(void) {
	// Configura como entrada y activa pull-up
	DHT_DDR &= ~(1 << DHT_BIT);
	DHT_PORT |= (1 << DHT_BIT);
	_delay_ms(1000); // Tiempo de estabilización del sensor
}

int8_t DHT11_Read(uint8_t *h, uint8_t *t) {
	uint8_t bits[5] = {0,0,0,0,0};
	uint8_t i, j = 0;

	// 1. SEÑAL DE INICIO
	DHT_DDR  |= (1 << DHT_BIT);  // Salida
	DHT_PORT &= ~(1 << DHT_BIT); // Low
	_delay_ms(20);
	
	DHT_PORT |= (1 << DHT_BIT);  // High
	_delay_us(30);
	DHT_DDR  &= ~(1 << DHT_BIT); // Entrada

	// 2. HANDSHAKE
	if (!wait_for_state(0)) return -1;
	if (!wait_for_state(1)) return -1;
	if (!wait_for_state(0)) return -1;

	// 3. LECTURA DE 40 BITS
	for (j = 0; j < 5; j++) {
		for (i = 0; i < 8; i++) {
			if (!wait_for_state(1)) return -1;
			_delay_us(40);
			
			if (DHT_PIN_REG & (1 << DHT_BIT)) {
				bits[j] |= (1 << (7 - i));
				if (!wait_for_state(0)) return -1;
			}
		}
	}

	// 4. CHECKSUM
	if (bits[4] == ((bits[0] + bits[1] + bits[2] + bits[3]) & 0xFF)) {
		*h = bits[0];
		*t = bits[2];
		return 1;
	}
	return 0;
}