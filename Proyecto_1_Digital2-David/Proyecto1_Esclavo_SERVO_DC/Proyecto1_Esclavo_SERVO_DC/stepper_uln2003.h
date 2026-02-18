/*
 * stepper_uln2003.h
 *
 * Created: 12/02/2026 07:18:12 p. m.
 *  Author: Admin
 */ 


#ifndef STEPPER_ULN2003_H_
#define STEPPER_ULN2003_H_

#include <stdint.h>

void Stepper_Init(void);
void Pump_Start(void);
void Pump_Stop(void);
void Stepper_Task(void);


#endif /* STEPPER_ULN2003_H_ */