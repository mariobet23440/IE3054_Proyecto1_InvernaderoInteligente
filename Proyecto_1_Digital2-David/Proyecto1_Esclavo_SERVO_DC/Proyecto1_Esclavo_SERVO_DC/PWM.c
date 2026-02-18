/*
 * pwm_init.c
 *
 * Created: 24/04/2025 12:23:08 p. m.
 *  Author: Admin
 */ 
#include "PWM3.h"

volatile uint8_t contador_PWM = 0;
volatile uint8_t limite_contador = 0;

void PWM3_Init(void){
	DDRD |= (1 << PIND5);	// PD5 como salida

	// Timer0 en modo normal con prescaler 64 (~976 Hz)
	TCCR0A = 0;
	TCCR0B = (1 << CS01) | (1 << CS00);

	TIMSK0 = (1 << TOIE0);  // Habilitar interrupción por overflow
	TCNT0 = 0;
}

void update_DutyCycle3(uint16_t dutyCycle){
	if (dutyCycle > 255) dutyCycle = 255;
	limite_contador = dutyCycle;
}

ISR(TIMER0_OVF_vect) {
	if (contador_PWM == 0) {
		PORTD |= (1 << PD5); // Encender LED
	}
	if (contador_PWM == limite_contador) {
		PORTD &= ~(1 << PD5); // Apagar LED
	}

	contador_PWM++;
	if (contador_PWM >= 255) {
		contador_PWM = 0;
	}
	TCNT0 = ;
}
