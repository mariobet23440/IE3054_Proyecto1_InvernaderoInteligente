/*
 * dc_motor.h
 *
 * Created: 16/02/2026 04:33:33 p. m.
 *  Author: Admin
 */ 


#ifndef DC_MOTOR_H_
#define DC_MOTOR_H_

#include <stdint.h>

void DC_Init(void);
void DC_On(void);
void DC_Off(void);
void DC_SetSpeed(uint8_t speed);


#endif /* DC_MOTOR_H_ */