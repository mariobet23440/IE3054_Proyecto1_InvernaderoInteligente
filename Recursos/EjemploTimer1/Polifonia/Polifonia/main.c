#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include "TIMER1.h"

// Definimos las frecuencias de las notas
#define NOTE_C4 261.5
#define NOTE_E4 360

// Sample Rate: La velocidad a la que "dibujamos" la onda (20 kHz es estándar)
#define SAMPLE_RATE 1

// Acumuladores de fase (Usamos 32 bits para mucha precisión)
volatile uint32_t phase1 = 0;
volatile uint32_t phase2 = 0;

// Incrementos de fase (determinan la frecuencia)
// Fórmula: (2^32 * Frecuencia) / Sample_Rate
uint32_t step1 = (uint32_t)((4294967296ULL * NOTE_C4) / SAMPLE_RATE);
uint32_t step2 = (uint32_t)((4294967296ULL * NOTE_E4) / SAMPLE_RATE);

int main(void)
{
	// 1. Configuramos el Timer 1 para que dispare una interrupción a 20 kHz
	// Modo CTC, Prescaler 1 para máxima velocidad
	Timer1Init(TIMER1_MODE4_CTC_OCR1A, TIMER1_PRESCALER1, TIMER1_INTERRUPTS_ENABLED, TIMER1_COMP_OUT_MODE0);
	
	// Calculamos el TOP para 20kHz: (16MHz / 20000) - 1 = 799
	OCR1A = 799;

	DDRB |= (1 << DDB5); // LED o Buzzer en PB5
	
	sei(); // Habilitar interrupciones globales

	while (1)
	{
		// Aquí podrías cambiar step1 y step2 para tocar diferentes acordes
	}
}

// Esta interrupción se ejecuta 20,000 veces por segundo
ISR(TIMER1_COMPA_vect)
{
	// Sumamos el incremento a la fase de cada nota
	phase1 += step1;
	phase2 += step2;

	// "Digital Mixer": Extraemos el bit más significativo de cada fase (onda cuadrada)
	// Usamos el operador XOR (^) para superponerlas en un solo pin
	uint8_t out1 = (phase1 >> 31);
	uint8_t out2 = (phase2 >> 31);

	if (out1 ^ out2) {
		PORTB |= (1 << PORTB5);
		} else {
		PORTB &= ~(1 << PORTB5);
	}
}
