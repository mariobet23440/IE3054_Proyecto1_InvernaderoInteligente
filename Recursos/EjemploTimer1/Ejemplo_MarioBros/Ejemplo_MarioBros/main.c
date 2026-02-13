#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "TIMER1.h"
#include "Notas.h"

// Melodía: Notas y duraciones (4=negra, 8=corchea, etc.)
int melody[] = {
	NOTE_E7, NOTE_E7, REST, NOTE_E7, REST, NOTE_C7, NOTE_E7, REST, NOTE_G7, REST, NOTE_G6, REST,
	NOTE_C7, NOTE_G6, NOTE_E6, NOTE_A6, NOTE_B6, NOTE_A6, NOTE_A6,
	NOTE_G6, NOTE_E7, NOTE_G7, NOTE_A7, NOTE_F7, NOTE_G7, NOTE_E7, NOTE_C7, NOTE_D7, NOTE_B6
};

int durations[] = {
	8, 8, 8, 8, 8, 8, 8, 8, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 8, 4,
	8, 8, 8, 4, 8, 4, 8, 8, 8, 4
};

void play_note(uint16_t frequency, uint16_t duration_ms) {
	if (frequency == REST) {
		// Desconectar el pin PB1 del Timer (Silencio)
		TCCR1A &= ~( (1 << COM1A1) | (1 << COM1A0) );
		} else {
		// Reconectar en modo Toggle y fijar frecuencia
		TCCR1A |= (1 << COM1A0);
		Timer1SetFrequency(frequency);
	}
	
	// Esperar la duración de la nota
	for(uint16_t i = 0; i < duration_ms; i++) _delay_ms(1);
	
	// Silencio breve entre notas para que no se "peguen"
	TCCR1A &= ~( (1 << COM1A1) | (1 << COM1A0) );
	_delay_ms(20);
}

int main(void) {
	// Inicializar Timer1: Modo CTC, Prescaler 8 (ideal para audio), Toggle OC1A
	Timer1Init(TIMER1_MODE4_CTC_OCR1A, TIMER1_PRESCALER8, TIMER1_INTERRUPTS_DISABLED, TIMER1_COMP_OUT_MODE1);
	
	// PB1 (OC1A) como salida
	DDRB |= (1 << DDB1);

	while (1) {
		for (int i = 0; i < sizeof(melody)/sizeof(melody[0]); i++) {
			// Calcular duración en ms (basado en un tempo base de 1000ms para una redonda)
			uint16_t noteDuration = 1000 / durations[i];
			play_note(melody[i], noteDuration);
		}
		_delay_ms(2000); // Esperar antes de repetir
	}
}