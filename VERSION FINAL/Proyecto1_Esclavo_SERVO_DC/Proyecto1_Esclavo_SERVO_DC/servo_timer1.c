/*
 * servo_timer1.c
 
 *
 * Created: 11/04/2025 01:35:52 p. m.
 *  Author: Admin
 */ 

#ifndef F_CPU
#define F_CPU 16000000UL
#endif
// pwm.c
#include <avr/io.h>
#include <stdint.h>

#include "servo_timer1.h"

#define SERVO_MIN_PULSE 1200   // 1 ms
#define SERVO_MAX_PULSE 4800   // 2 ms


void Servo_Init(void)
{
	DDRB |= (1 << PB1);   // D9

	TCCR1A = (1 << COM1A1) | (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);

	ICR1 = 39999;         // 50 Hz
	OCR1A = SERVO_MIN_PULSE;
}

void Servo_SetAngle(uint8_t angle)
{
	if (angle > 180)
	angle = 180;

	OCR1A = SERVO_MIN_PULSE +
	((uint32_t)angle * (SERVO_MAX_PULSE - SERVO_MIN_PULSE)) / 180;
}
