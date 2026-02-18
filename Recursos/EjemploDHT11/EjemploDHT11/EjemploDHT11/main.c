#define F_CPU 16000000UL  // Frecuencia del reloj (16MHz para Arduino Uno)
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>       // Necesario para itoa()
#include "DHT11.h"        // Tu nueva librería flexible

// --- CONFIGURACIÓN UART ---
#define BAUD 9600
#define MY_UBRR F_CPU/16/BAUD-1

void UART_Init(unsigned int ubrr) {
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1 << TXEN0); // Habilitar Transmisión
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 bits de datos, 1 bit de parada
}

void UART_SendChar(char data) {
	while (!(UCSR0A & (1 << UDRE0))); // Esperar a que el buffer esté vacío
	UDR0 = data;
}

void UART_SendString(char *str) {
	while (*str) {
		UART_SendChar(*str++);
	}
}

// --- PROGRAMA PRINCIPAL ---
int main(void) {
	// 1. Inicializar periféricos
	UART_Init(MY_UBRR);
	DHT11_Init(); // Configura el pin y espera la estabilización del sensor
	
	// Variables para almacenar los datos
	uint8_t humedad = 0;
	uint8_t temperatura = 0;
	char buffer[10]; // Buffer para convertir números a texto
	
	UART_SendString("--- Sistema de Monitoreo DHT11 Iniciado ---\r\n");

	while (1) {
		// 2. Intentar leer el sensor
		int8_t status = DHT11_Read(&humedad, &temperatura);

		if (status == 1) {
			// LECTURA EXITOSA
			UART_SendString("Humedad: ");
			itoa(humedad, buffer, 10);
			UART_SendString(buffer);
			UART_SendString("% | Temp: ");
			itoa(temperatura, buffer, 10);
			UART_SendString(buffer);
			UART_SendString(" C\r\n");
		}
		else if (status == 0) {
			// ERROR DE CHECKSUM
			UART_SendString("[!] Error: Checksum incorrecto (Ruido en la linea).\r\n");
		}
		else {
			// ERROR DE TIMEOUT
			UART_SendString("[!] Error: Sensor no responde (Verificar conexion).\r\n");
		}

		// 3. Esperar 2 segundos antes de la siguiente lectura
		// El DHT11 es un sensor lento, si lo lees muy rápido dará error.
		_delay_ms(2000);
	}

	return 0;
}