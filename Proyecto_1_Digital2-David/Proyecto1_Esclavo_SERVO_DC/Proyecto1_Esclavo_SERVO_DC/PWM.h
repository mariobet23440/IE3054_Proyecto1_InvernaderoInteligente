/*
 * pwm_init.h
 *
 * Created: 24/04/2025 12:23:28 p. m.
 *  Author: Admin
 */ 


#ifndef PWM3_H_
#define PWM3_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

void PWM3_Init(void);
void update_DutyCycle3(uint16_t dutyCycle);

#endif
