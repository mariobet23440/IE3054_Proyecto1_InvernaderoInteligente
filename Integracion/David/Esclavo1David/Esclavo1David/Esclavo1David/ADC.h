/*
 * ADC.h
 *
 * Created: 15/02/2026 09:57:24 p. m.
 *  Author: David Carranza
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>

void ADC_Init(void);
uint16_t ADC_Read(uint8_t channel);

#endif /* ADC_H_ */