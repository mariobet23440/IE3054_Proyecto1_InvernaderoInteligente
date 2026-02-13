#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h> // Necesario para rand()
#include "TIMER1.h"

// --- Función para el Bombo (Kick Drum) ---
// Es una frecuencia que cae rápidamente simulando el golpe del parche
void play_kick() {
	TCCR1A |= (1 << COM1A0); // Conectar sonido
	
	// Barrido de frecuencia descendente
	for (uint16_t freq = 200; freq > 40; freq -= 5) {
		Timer1SetFrequency(freq);
		_delay_ms(2); // Duración del descenso
	}
	
	TCCR1A &= ~(1 << COM1A0); // Silencio
}

// --- Función para el Redoblante (Snare Drum) ---
// Usa frecuencias aleatorias muy rápidas para simular ruido (noise)
void play_snare() {
	TCCR1A |= (1 << COM1A0); // Conectar sonido
	
	for (uint8_t i = 0; i < 60; i++) {
		// Generar una frecuencia aleatoria entre 400 y 1200 Hz
		uint16_t noise = (rand() % 800) + 400;
		Timer1SetFrequency(noise);
		_delay_us(800); // Cambio de frecuencia extremadamente rápido
	}
	
	TCCR1A &= ~(1 << COM1A0); // Silencio
}

int main(void)
{
	// Inicializar Timer1 en Modo CTC con Prescaler 8
	// Usamos Prescaler 8 para tener alta resolución en el barrido de frecuencias
	Timer1Init(TIMER1_MODE4_CTC_OCR1A, TIMER1_PRESCALER8, TIMER1_INTERRUPTS_DISABLED, TIMER1_COMP_OUT_MODE1);
	
	DDRB |= (1 << DDB1); // Pin PB1 (OC1A) como salida para el buzzer

	while (1)
	{
		// Ritmo básico: Bombo - Redoblante - Bombo - Redoblante
		play_kick();
		_delay_ms(300);
		
		play_snare();
		_delay_ms(300);
		
		play_kick();
		_delay_ms(150);
		
		play_kick();
		_delay_ms(150);
		
		play_snare();
		_delay_ms(600);
	}
}