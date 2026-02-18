/*
 * dc_motor.c
 *
 * Created: 16/02/2026 04:33:46 p. m.
 *  Author: Admin
 */ 

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include "dc_motor.h"

#define IN1		PD5
#define IN2		PD6
#define ENA		PD7	// PWM

void DC_Init(void)
{
	DDRD |= (1 << IN1) | (1 << IN2) | (1 << ENA);
	
	// Configurar Timer 2 para PWM rapido
	TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
	TCCR2B = (1 << CS21);	//Prescaler 8
	
	OCR2B = 0;	// Motor Apagado
}

void DC_On(void)
{
	PORTD |= (1 << IN1);
	PORTD &= ~(1 << IN2);
	OCR2B = 200;	// Velocidad fija
}

void DC_Off(void)
{
	OCR2B = 0;
}

void DC_SetSpeed(uint8_t speed)
{
	OCR2B = speed;
}