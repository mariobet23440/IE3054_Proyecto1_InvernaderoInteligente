#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#define DHT_PIN     PD2
#define DHT_PORT    PORTD
#define DHT_DDR     DDRD
#define DHT_PIN_REG PIND

// --- UART ---
void UART_Init(unsigned int ubrr) {
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_SendString(char *str) {
	while (*str) {
		while (!(UCSR0A & (1 << UDRE0)));
		UDR0 = *str++;
	}
}

// --- LÓGICA DEL SENSOR ---
uint8_t wait_for_state(uint8_t state) {
	uint8_t timeout = 0;
	while (((DHT_PIN_REG & (1 << DHT_PIN)) ? 1 : 0) != state) {
		_delay_us(1);
		timeout++;
		if (timeout > 200) return 0;
	}
	return 1;
}

// Función modificada: Devuelve 1 si la lectura fue exitosa, 0 si falló.
// Los valores se guardan en las direcciones de memoria de 'h' y 't'.
int8_t ReadDHT11(uint8_t *h, uint8_t *t) {
	uint8_t bits[5] = {0,0,0,0,0};
	uint8_t i, j = 0;

	// 1. SEÑAL DE INICIO
	DHT_DDR |= (1 << DHT_PIN);
	DHT_PORT &= ~(1 << DHT_PIN);
	_delay_ms(20);
	DHT_PORT |= (1 << DHT_PIN);
	_delay_us(30);
	DHT_DDR &= ~(1 << DHT_PIN);

	// 2. HANDSHAKE
	if (!wait_for_state(0)) return -1;
	if (!wait_for_state(1)) return -1;
	if (!wait_for_state(0)) return -1;

	// 3. LEER 40 BITS
	for (j = 0; j < 5; j++) {
		for (i = 0; i < 8; i++) {
			if (!wait_for_state(1)) return -1;
			_delay_us(40);
			if (DHT_PIN_REG & (1 << DHT_PIN)) {
				bits[j] |= (1 << (7 - i));
				if (!wait_for_state(0)) return -1;
			}
		}
	}

	// 4. VERIFICAR CHECKSUM Y ASIGNAR VALORES
	if (bits[4] == ((bits[0] + bits[1] + bits[2] + bits[3]) & 0xFF)) {
		*h = bits[0]; // Guardamos humedad en la variable externa
		*t = bits[2]; // Guardamos temperatura en la variable externa
		return 1;     // Lectura exitosa
	}
	
	return 0; // Error de Checksum
}

int main(void) {
	UART_Init(103);
	
	// VARIABLES PARA ALMACENAR LOS DATOS
	uint8_t humedad = 0;
	uint8_t temperatura = 0;
	char buffer[10];

	UART_SendString("Sistema Iniciado...\r\n");

	while (1) {
		// Pasamos la dirección de memoria (&) de nuestras variables
		if (ReadDHT11(&humedad, &temperatura) == 1) {
			
			// Ahora puedes usar 'humedad' y 'temperatura' para lo que quieras
			// Ejemplo: Enviarlos por Serial
			UART_SendString("H: ");
			itoa(humedad, buffer, 10);
			UART_SendString(buffer);
			
			UART_SendString("% | T: ");
			itoa(temperatura, buffer, 10);
			UART_SendString(buffer);
			UART_SendString(" C\r\n");

			} else {
			UART_SendString("Error de lectura\r\n");
		}

		_delay_ms(2000); // El DHT11 es lento, no leer más rápido que esto
	}
}