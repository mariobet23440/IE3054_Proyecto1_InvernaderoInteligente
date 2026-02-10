/*
 * TIMER1.c
 * Librería para utilizar TIMER1 en ATMEGA328P
 * Adaptable para otros microcontroladores
 *
 * Creada: 10/02/2026 15:06:40
 * Autor: mario
 */ 

#include "TIMER1.h"

// Inicialización de TIMER1
// Los prefijos s denotan "selector"
void Timer1Init(TIMER1_MODE sMode, TIMER1_PRESCALERS sPrescaler, TIMER1_EN_INTTERUPTS enInterrupts, TIMER1_COMPARE_OUTPUT_MODE sComOutMode)
{
	// Aplicar máscara de bits a selectores de modo y prescaler
	uint8_t mode = sMode & 0x0F;			// Obtener últimos 4 bits de mode
	uint8_t prescaler = sPrescaler & 0x07;	// Obtener últimos 3 bits de prescaler
	uint8_t cout_mode = (sComOutMode & 0x03) << 6;
	
	// Aislar pares de bits 1-0 y 3-2
	uint8_t mode10 = mode & 0x03;
	uint8_t mode32 = (mode & 0x0C) << 3;
	
	TCCR1A |= mode10 | cout_mode;
	TCCR1B |= mode32 | prescaler;
	
	if(enInterrupts == TIMER1_INTERRUPTS_ENABLED) TIMSK1 |= (1 << ICIE1) | (1 << OCIE1B) | (1 << OCIE1A) | (1 << TOIE1);
}

// Establecer frecuencia, asumiendo periodo de
static TIMER1_MODE getTimer1WGMode(void)
{
	uint8_t mode10 = TCCR1A & ((1 << WGM11) | (1 << WGM10));
	uint8_t mode32 = (TCCR1B & ((1 << WGM13) | (1 << WGM12))) >> 1;
	TIMER1_MODE wgMode = (TIMER1_MODE) mode10 | mode32;
	return 	wgMode;
}

// Establecer frecuencia, asumiendo frecuencia de reloj de 16 MHz
void Timer1SetFrequency(uint16_t frequency)
{
	// Determinar prescaler
	uint8_t sPrescaler = TCCR1B & 0x03;
	uint16_t prescaler = 0;
	
	switch(sPrescaler)
	{
		case 1:  prescaler = 1;		break;
		case 2:  prescaler = 8;		break;
		case 3:  prescaler = 64;	break;
		case 4:  prescaler = 256;	break;
		case 5:  prescaler = 1024;	break;
		default: prescaler = 1;		break;
	}
	
	
	// Variables temporales para frecuencias
	uint16_t temp_CTC		= F_CPU / (2*prescaler * frequency) - 1;
	uint16_t temp_FASTPWM	= F_CPU / (prescaler * frequency) - 1;
	uint16_t temp_PWM_PC	= F_CPU / (2*prescaler * frequency);
	uint16_t temp_PWM_PCF	= F_CPU / (2*prescaler * frequency);
	
	TIMER1_MODE wgMode = (TIMER1_MODE) getTimer1WGMode();
	switch(wgMode)
	{
		case TIMER1_MODE4_CTC_OCR1A:		OCR1A	= temp_CTC;		break;	
		case TIMER1_MODE8_PWM_PCF_ICR1:		ICR1	= temp_PWM_PCF;	break;
		case TIMER1_MODE9_PWM_PCF_OCR1A:	OCR1A	= temp_PWM_PCF;	break;
		case TIMER1_MODE10_PWM_PC_ICR1:		ICR1	= temp_PWM_PC;	break;
		case TIMER1_MODE11_PWM_PC_OCR1A:	OCR1A	= temp_PWM_PC;	break;
		case TIMER1_MODE12_CTC_ICR1:		ICR1	= temp_CTC;		break;
		case TIMER1_MODE14_FASTPWM_ICR1A:	ICR1	= temp_FASTPWM;	break;
		case TIMER1_MODE15_FASTPWM_OCR1A:	OCR1A	= temp_FASTPWM; break;
		default:	OCR1A = 0xFFFF;			ICR1	= 0XFFFF;		break;
	}
	
}

