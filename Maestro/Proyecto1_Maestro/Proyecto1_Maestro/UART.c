/*
 * LIBRERÍA UART (v3.0)
 * UART.c
 *
 * Creada: 27/01/2026 07:10:21
 * Author: Mario Alejandro Betancourt Franco
 * Actualización: Se construyeron presets para frecuencias de reloj de 16 MHz
 
 */ 

#include "UART.h"

#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

void UART_Init(UART_BAUD_CONFIG ubrr_value, UART_INTERRUPT_CONFIG interrupts_en)
{
	// Configurar pines de salida
	DDRD |=  (1 << DDD1);   // Salida (TX)
	DDRD &= ~(1 << DDD0);   // Entrada (RX)

	// Reset de registros de control
	UCSR0A = 0;
	UCSR0B = 0;
	UCSR0C = 0;
	
	// Activar modo double speed para los presets
	switch(ubrr_value)
	{
		case UART_BAUD_4800_16MHZ:	 UCSR0A |= (1 << U2X0); break;
		case UART_BAUD_9600_16MHZ:	 UCSR0A |= (1 << U2X0); break;
		case UART_BAUD_19200_16MHZ:  UCSR0A |= (1 << U2X0); break;
		case UART_BAUD_38400_16MHZ:  UCSR0A |= (1 << U2X0); break;
		case UART_BAUD_57600_16MHZ:	 UCSR0A |= (1 << U2X0); break;
		case UART_BAUD_115200_16MHZ: UCSR0A |= (1 << U2X0); break;
	}
	
	// Poner valor de UBRR0;
	UBRR0 = ubrr_value; 

	// Configuración de formato: 8N1 (Mensaje de 8 bits, 1 bit de stop)
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	// Habilitar Transmisor y Receptor
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);

	// Configurar interrupciones si se solicitaron
	if (interrupts_en & 1) {
		UCSR0B |= (1 << RXCIE0);
	}
}




// Enviar un carácter
void UART_sendChar(char c) {
	while (!(UCSR0A & (1 << UDRE0)));  // Espera buffer libre
	UDR0 = c;
}

// Enviar una cadena de texto
void UART_sendString(const char* str) {
	while (*str) UART_sendChar(*str++);
}

unsigned char UART_Receive(void)
{
	/* Wait for data to be received */
	while (!(UCSR0A & (1<<RXC0)));
	/* Get and return received data from buffer */
	return UDR0;
}