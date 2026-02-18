/*
 * Maestro.c
 *
 * Created: 12/02/2026 07:37:51 p. m.
 * Author : David Carranza
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "I2C.h"
#include "UART.h"
#include "HD44780_4b.h"

/************************************************************************/
/*                        DIRECCIONES I2C                               */
/************************************************************************/

#define SLAVE_ACTUATORS_ADDR  0x30	// Nano 1
#define SLAVE_ENV_ADDR		  0x31	// Nano 2
#define TSL2561_ADDR          0x39	// Sensor de Luz

/************************************************************************/
/* COMANDOS NANO 1                                                      */
/************************************************************************/

#define CMD_PUMP_START	0x20
#define CMD_PUMP_STOP	0x21
#define CMD_SERVO_OPEN	0x10
#define CMD_SERVO_CLOSE	0x11

/************************************************************************/
/* COMANDOS NANO 2                                                      */
/************************************************************************/

#define CMD_READ_SOIL	0x40
#define CMD_FAN_ON		0x41
#define CMD_FAN_OFF		0X42
#define CMD_FAN_PWM		0x43

/************************************************************************/
/* COMANDOS SENSOR LUZ                                                                     */
/************************************************************************/

#define TSL2561_CMD_BIT       0x80
#define TSL2561_WORD_BIT      0x20
#define TSL2561_REG_CONTROL   0x00
#define TSL2561_REG_DATA0LO   0x0C
#define CMD_POWER_UP          (TSL2561_CMD_BIT | TSL2561_REG_CONTROL)
#define CMD_READ_DATA         (TSL2561_CMD_BIT | TSL2561_WORD_BIT | TSL2561_REG_DATA0LO)

/************************************************************************/
/* PARAMETROS                                                           */
/************************************************************************/

#define SOIL_THRESHOLD   150
#define RX_BUFFER_SIZE   32
#define SOIL_RAW_DRY   255
#define SOIL_RAW_WET   160

/************************************************************************/
/* VARIABLES UART                                                       */
/************************************************************************/

volatile char rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_index = 0;
volatile uint8_t command_ready = 0;

/************************************************************************/
/* LCD 4 BITS                                                                    */
/************************************************************************/

LCD_4b lcd = {
	.rs = { &PORTD, &DDRD, PORTD2 },
	.e  = { &PORTD, &DDRD, PORTD3 },
	.d4 = { &PORTD, &DDRD, PORTD5 },
	.d5 = { &PORTD, &DDRD, PORTD6 },
	.d6 = { &PORTD, &DDRD, PORTD7 },
	.d7 = { &PORTB, &DDRB, PORTB0 }
};

/************************************************************************/
/* FUNCIONES I2C                                                        */
/************************************************************************/

void TSL2561_Init_Sensor(void) {
	if (I2C_MasterStart()) {
		if (I2C_Master_Write((TSL2561_ADDR << 1) | I2C_WRITE) == 0x18) {
			I2C_Master_Write(CMD_POWER_UP);
			I2C_Master_Write(0x03); // Encender
		}
		I2C_MasterStop();
	}
}

uint16_t TSL2561_Read_Luminosity(void) {
	uint8_t lo = 0, hi = 0;
	if (I2C_MasterStart()) {
		I2C_Master_Write((TSL2561_ADDR << 1) | I2C_WRITE);
		I2C_Master_Write(CMD_READ_DATA);
		I2C_MasterRepeatedStart();
		I2C_Master_Write((TSL2561_ADDR << 1) | I2C_READ);
		I2C_MasterRead(&lo, I2C_ACK);
		I2C_MasterRead(&hi, I2C_NACK);
		I2C_MasterStop();
	}
	return (uint16_t)((hi << 8) | lo);
}


void Pump_Start_Command(void)
{
	I2C_MasterStart();
	I2C_Master_Write((SLAVE_ACTUATORS_ADDR<<1)|I2C_WRITE);
	I2C_Master_Write(CMD_PUMP_START);
	I2C_MasterStop();
}

void Pump_Stop_Command(void)
{
	I2C_MasterStart();
	I2C_Master_Write((SLAVE_ACTUATORS_ADDR<<1)|I2C_WRITE);
	I2C_Master_Write(CMD_PUMP_STOP);
	I2C_MasterStop();
}

void Servo_Open(void)
{
	I2C_MasterStart();
	I2C_Master_Write((SLAVE_ACTUATORS_ADDR<<1)|I2C_WRITE);
	I2C_Master_Write(CMD_SERVO_OPEN);
	I2C_MasterStop();
}

void Servo_Close(void)
{
	I2C_MasterStart();
	I2C_Master_Write((SLAVE_ACTUATORS_ADDR<<1)|I2C_WRITE);
	I2C_Master_Write(CMD_SERVO_CLOSE);
	I2C_MasterStop();
}

void Fan_On(void)
{
	I2C_MasterStart();
	I2C_Master_Write((SLAVE_ENV_ADDR<<1)|I2C_WRITE);
	I2C_Master_Write(CMD_FAN_ON);
	I2C_MasterStop();
}

void Fan_Off(void)
{
	I2C_MasterStart();
	I2C_Master_Write((SLAVE_ENV_ADDR<<1)|I2C_WRITE);
	I2C_Master_Write(CMD_FAN_OFF);
	I2C_MasterStop();
}

void Fan_SetSpeed(uint8_t speed)
{
	I2C_MasterStart();
	I2C_Master_Write((SLAVE_ENV_ADDR<<1)|I2C_WRITE);
	I2C_Master_Write(CMD_FAN_PWM);
	I2C_Master_Write(speed);
	I2C_MasterStop();
}


uint8_t Read_Soil(void)
{
	uint8_t soil_value = 0;

	if(I2C_MasterStart())
	{
		I2C_Master_Write((SLAVE_ENV_ADDR<<1)|I2C_WRITE);
		I2C_Master_Write(CMD_READ_SOIL);

		I2C_MasterRepeatedStart();
		I2C_Master_Write((SLAVE_ENV_ADDR<<1)|I2C_READ);
		I2C_MasterRead(&soil_value, I2C_NACK);

		I2C_MasterStop();
	}

	return soil_value;
}



/************************************************************************/
/* PROCESAMIENTO UART                                                   */
/************************************************************************/

void Process_Command(void)
{
	// Ejemplos:
	// P1 -> Start bomba
	// P0 -> Stop bomba
	// S1 -> Servo abrir
	// S0 -> Servo cerrar
	// Q  -> Sensor humedad suelo
	// L  -> Sensor luz ambiente
	
	if(rx_buffer[0] == 'Q')   // Consulta humedad
	{
		
		uint8_t soil = Read_Soil();

		char buffer[40];
		uint8_t percentage;

		if(soil >= SOIL_RAW_DRY)
		percentage = 0;
		else if(soil <= SOIL_RAW_WET)
		percentage = 100;
		else
		{
			percentage = 100 -
			((soil - SOIL_RAW_WET) * 100) /
			(SOIL_RAW_DRY - SOIL_RAW_WET);
		}
		
		sprintf(buffer, "Soil: %d (%d%%)\r\n", soil, percentage);
		UART_SendString(buffer);

		if(percentage > 40)
		UART_SendString("No necesita agua\r\n");
		else
		UART_SendString("Necesita agua\r\n");
	}
	
	else if(rx_buffer[0] == 'L') 
	{ // Comando Luminosidad
		char buffer[40]
		uint16_t lux = TSL2561_Read_Luminosity();
		sprintf(buffer, "Luminosidad: %u lux\r\n", lux);
		UART_SendString(buffer);
	}
	
	else if(rx_buffer[0] == 'P')
	{
		if(rx_buffer[1] == '1')
		{
			Pump_Start_Command();
			UART_SendString("Pump START\r\n");
		}
		else if(rx_buffer[1] == '0')
		{
			Pump_Stop_Command();
			UART_SendString("Pump STOP\r\n");
		}
	}
	
	else if(rx_buffer[0] == 'S')
	{
		if(rx_buffer[1] == '1')
		{
			Servo_Open();
			UART_SendString("Servo OPEN\r\n");
		}
		else if(rx_buffer[1] == '0')
		{
			Servo_Close();
			UART_SendString("Servo CLOSE\r\n");
		}
	}
	
	else if(rx_buffer[0] == 'F')
	{
		if(rx_buffer[1] == '1')
		{
			Fan_On();
			UART_SendString("Fan ON\r\n");
		}
		else if(rx_buffer[1] == '0')
		{
			Fan_Off();
			UART_SendString("Fan OFF\r\n");
		}
	}
	
	else if(rx_buffer[0] == 'V')
	{
		uint8_t speed = atoi((char*)&rx_buffer[1]);
		if(speed > 255) speed = 255;

		Fan_SetSpeed(speed);

		char buffer[40];
		sprintf(buffer, "Fan speed: %d\r\n", speed);
		UART_SendString(buffer);
	}

}


/************************************************************************/
/* ISR                                                                  */
/************************************************************************/

ISR(USART_RX_vect)
{
	char c = UDR0;

	if(c == '\n' || c == '\r')
	{
		rx_buffer[rx_index] = '\0';
		command_ready = 1;
		rx_index = 0;
	}
	else
	{
		if(rx_index < RX_BUFFER_SIZE - 1)
		rx_buffer[rx_index++] = c;
	}
}

/************************************************************************/
/* MAIN                                                                 */
/************************************************************************/

int main(void)
{
	I2C_MasterInit(100000UL, 1);
	UART_Init(UART_BAUD_9600_16MHZ, UART_INTERRUPTS_ENABLED);
	LCD_Init_4b(&lcd);
	
	// Inicializar TSL2561
	I2C_MasterStart();
	I2C_Master_Write((TSL2561_ADDR << 1) | I2C_WRITE);
	I2C_Master_Write(0x80 | 0x00); // Control Register
	I2C_Master_Write(0x03);        // Power Up
	I2C_MasterStop();

	sei();   // Habilitar interrupciones globales

	LCD_Clear(&lcd);
	LCD_SetCursor(&lcd, 0, 0);
	LCD_WriteString(&lcd, "INVERNADERO OK");
	
	UART_SendString("Sistema listo\r\n");
	
	//uint8_t current_hum = 0;
	char lcd_buf[17];

	while(1)
	{
		// Comandos UART
		if(command_ready)
		{
			Process_Command();
			command_ready = 0;
		}
		
		// B. Actualizar datos de sensores cada cierto tiempo (sin usar delay largo)
		static uint16_t refresh_timer = 0;
		if(refresh_timer++ > 500) 
		
		{ 
			// Aproximadamente cada 500ms
			uint16_t current_lux = TSL2561_Read_Luminosity();
			uint8_t soil = Read_Soil();

			uint8_t percentage;

			if(soil >= SOIL_RAW_DRY)
			percentage = 0;
			else if(soil <= SOIL_RAW_WET)
			percentage = 100;
			else
			{
				percentage = 100 -
				((soil - SOIL_RAW_WET) * 100) /
				(SOIL_RAW_DRY - SOIL_RAW_WET);
			}

			
			// Actualizar LCD Fila 2
			LCD_SetCursor(&lcd, 0, 10);
			sprintf(lcd_buf, "L: %5u lux  ", current_lux);
			LCD_WriteString(&lcd, lcd_buf);
			
			LCD_SetCursor(&lcd, 0, 1);
			sprintf(lcd_buf, "H: %3d%%  ", percentage);
			LCD_WriteString(&lcd, lcd_buf);
			
			refresh_timer = 0;
		}
		
		_delay_ms(1);
	}
}
