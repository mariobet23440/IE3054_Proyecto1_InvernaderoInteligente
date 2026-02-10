/*
 * TIMER1.h  (REV. B - Corregida)
 *
 * Created: 10/02/2026 15:06:53
 * Author: mario
 */ 

#ifndef TIMER1_H_
#define TIMER1_H_

#include <avr/io.h>
#include <stdio.h>

// MODOS DE GENERACIÓN DE ONDA
// Números de 4 bits que representan
// WGM13 - WGM12 - WGM11 - WGM10
typedef enum
{
    TIMER1_MODE0_NORMAL         = 0,
    TIMER1_MODE1_PWM_PC8BIT     = 1,
    TIMER1_MODE2_PWM_PC9BIT     = 2,
    TIMER1_MODE3_PWM_PC10BIT    = 3,
    TIMER1_MODE4_CTC_OCR1A      = 4,  // RECOMENDADO PARA TEMPORIZADOR EXACTO
    TIMER1_MODE5_FASTPWM_8BIT   = 5,
    TIMER1_MODE6_FASTPWM_9BIT   = 6,
    TIMER1_MODE7_FASTPWM_10BIT  = 7,
    TIMER1_MODE8_PWM_PCF_ICR1   = 8,
    TIMER1_MODE9_PWM_PCF_OCR1A  = 9,
    TIMER1_MODE10_PWM_PC_ICR1   = 10,
    TIMER1_MODE11_PWM_PC_OCR1A  = 11,
    TIMER1_MODE12_CTC_ICR1      = 12,
    TIMER1_MODE14_FASTPWM_ICR1A = 14,
    TIMER1_MODE15_FASTPWM_OCR1A = 15
} TIMER1_MODE;

// PRESCALERS DE TIMER1
// Bits representan CS12 - CS11 - CS10
typedef enum
{
    TIMER1_NO_CLOCK         = 0,
    TIMER1_PRESCALER1       = 1,
    TIMER1_PRESCALER8       = 2,
    TIMER1_PRESCALER64      = 3,
    TIMER1_PRESCALER256     = 4, // Ideal para tiempos humanos (segundos)
    TIMER1_PRESCALER1024    = 5,
    TIMER1_EXT_CLK_FALLING  = 6,
    TIMER1_EXT_CLK_RISING   = 7
} TIMER1_PRESCALERS;

typedef enum
{
    TIMER1_INTERRUPTS_DISABLED,
    TIMER1_INTERRUPTS_ENABLED,
} TIMER1_EN_INTERRUPTS;

typedef enum
{
    TIMER1_COMP_OUT_MODE0 = 0, // Operación normal, pines desconectados
    TIMER1_COMP_OUT_MODE1 = 1,
    TIMER1_COMP_OUT_MODE2 = 2,
    TIMER1_COMP_OUT_MODE3 = 3
} TIMER1_COMPARE_OUTPUT_MODE;

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

void Timer1Init(TIMER1_MODE sMode, TIMER1_PRESCALERS sPrescaler, TIMER1_EN_INTTERUPTS enInterrupts, TIMER1_COMPARE_OUTPUT_MODE sComOutMode);
void Timer1SetFrequency(uint16_t frequency);

#endif /* TIMER1_H_ */