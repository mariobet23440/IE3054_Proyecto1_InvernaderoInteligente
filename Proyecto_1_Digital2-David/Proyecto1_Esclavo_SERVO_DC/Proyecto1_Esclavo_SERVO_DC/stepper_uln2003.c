/*
 * stepper_uln2003.c
 *
 * Created: 12/02/2026 07:21:00 p. m.
 *  Author: Admin
 */ 

#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include <avr/io.h>
#include "stepper_uln2003.h"

#define IN1 PD4
#define IN2 PD5
#define IN3 PD6
#define IN4 PD7

static const uint8_t sequence[8] =
{
	0b00010000,
	0b00110000,
	0b00100000,
	0b01100000,
	0b01000000,
	0b11000000,
	0b10000000,
	0b10010000
};

static uint8_t step_index = 0;
static uint8_t pump_running = 0;
static uint16_t step_counter = 0;

void Stepper_Init(void)
{
	DDRD |= (1<<IN1)|(1<<IN2)|(1<<IN3)|(1<<IN4);
}

void Pump_Start(void)
{
	pump_running = 1;
}

void Pump_Stop(void)
{
	pump_running = 0;
	PORTD &= 0x0F;
}

void Stepper_Task(void)
{
	if(pump_running)
	{
		step_counter++;

		if(step_counter >= 2000)   // Ajustar velocidad
		{
			step_counter = 0;

			step_index = (step_index + 1) % 8;
			PORTD = (PORTD & 0x0F) | sequence[step_index];
		}
	}
}
