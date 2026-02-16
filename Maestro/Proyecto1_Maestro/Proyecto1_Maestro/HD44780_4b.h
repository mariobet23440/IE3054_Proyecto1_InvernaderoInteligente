/************************************************************************/
/*	LIBRERÍA HD44780 (Ver 2.0) - INCLUDE FILE
	CREADA POR: Mario Alejandro Betancourt Franco (23440)
	DESCRIPCIÓN: Librería para uC HD44780 para pantallas LCD
				 Modo de 4 bits.
	PARA UTILIZARLA: 
			1. Definir la struct LCD_PIN en el código principal
			2. Asignar direcciones de puertos, pines y ddrs
			3. Inicializar con LCD_Init_4b
			4. Colocar cursor con LCD_SetCursor
			5. Escribir cadena de texto con LCD_WriteString.
	ÚLTIMA ACTUALIZACIÓN: 12/02/2026						            */
/************************************************************************/

#ifndef HD44780_4B_H_
#define HD44780_4B_H_

#include <avr/io.h>
#include <stdint.h>

/* Pin genérico */
typedef struct {
	volatile uint8_t *port;
	volatile uint8_t *ddr;
	uint8_t pin;
} LCD_Pin;

/* LCD en modo 4 bits */
typedef struct {
	LCD_Pin rs;
	LCD_Pin e;
	LCD_Pin d4;
	LCD_Pin d5;
	LCD_Pin d6;
	LCD_Pin d7;
} LCD_4b;

/* API pública */
void LCD_Init_4b(LCD_4b *lcd);
void LCD_Command(LCD_4b *lcd, uint8_t cmd);
void LCD_WriteChar(LCD_4b *lcd, char c);
void LCD_WriteString(LCD_4b *lcd, const char *str);
void LCD_SetCursor(LCD_4b *lcd, uint8_t col, uint8_t row);
void LCD_Clear(LCD_4b *lcd);

#endif /* HD44780_4B_H_ */
