#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h> // 1. Obligatorio para usar ISR
#include "UART.h"

// Variable global para guardar el dato (marcada como volatile para que el compilador no la optimice)
volatile char datoRecibido = 0;
volatile uint8_t nuevoDato = 0;

int main(void)
{
	// 2. Inicializar con interrupciones habilitadas según tu Enum
	UART_Init(UART_BAUD_9600_16MHZ, UART_INTERRUPTS_ENABLED);
	
	// 3. Habilitar interrupciones globales
	sei();
	
	UART_sendString("--- UART con Interrupcion ---\r\n");

	while (1)
	{
		// El main está libre para otras tareas.
		// Solo revisamos si la interrupción nos dejó un dato nuevo.
		if (nuevoDato) {
			UART_sendString("Recibido por ISR: ");
			UART_sendChar(datoRecibido);
			UART_sendString("\r\n");
			
			nuevoDato = 0; // Limpiar bandera
		}
		
		// Simular que el micro está haciendo otra cosa
		_delay_ms(10);
	}
}

// 4. Definir el vector de interrupción para Recepción UART
// USART_RX_vect es el nombre estándar para el ATmega328P
ISR(USART_RX_vect)
{
	// Leemos el registro directamente (o podrías usar UDR0)
	// No usamos UART_Receive() aquí porque esa función tiene un 'while'
	// y dentro de una ISR nunca debemos esperar.
	datoRecibido = UDR0;
	nuevoDato = 1; // Avisar al main que llegó algo
}