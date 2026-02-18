/*
 * timer1_config.c
 *
 * Created: 24/04/2025 12:36:11 p. m.
 *  Author: David Carranza
 */ 

#include <avr/io.h>
#include "timer1_config.h"

void Timer1_Config_Init(void) {
	TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11); // PWM
	TCCR1B = (1 << WGM13) | (1 << CS11); // Prescaler 8
	ICR1 = 20000; // 20ms
}
