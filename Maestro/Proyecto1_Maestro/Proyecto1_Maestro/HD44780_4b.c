#define F_CPU 16000000
#include "HD44780_4b.h"
#include <util/delay.h>

/* ====== Funciones internas ====== */

// DEFINIR UN PIN LCD COMO UNA SALIDA Y APAGARLO
static void lcd_pin_output(LCD_Pin *p){
	*(p->ddr) |= (1 << p->pin);
	*(p->port) &= ~(1 << p->pin);
}

// ENCENDER O APAGAR UN PIN
static void lcd_pin_write(LCD_Pin *p, uint8_t val){
	if (val) *(p->port) |= (1 << p->pin);
	else *(p->port) &= ~(1 << p->pin);
}

// GENERAR UN PULSO EN PIN E
static void lcd_pulse_enable(LCD_4b *lcd){
	lcd_pin_write(&lcd->e, 1); // E = 1
	_delay_us(1);			   // Esperar 1 us
	lcd_pin_write(&lcd->e, 0); // E = 0
	_delay_us(100);			   // Esperar 100 us
}

// ESCRIBIR UN NIBBLE EN LCD
static void lcd_write_nibble(LCD_4b *lcd, uint8_t nibble)
{
	// Escribir nibble bits d4-d7
	lcd_pin_write(&lcd->d4, (nibble >> 0) & 1);
	lcd_pin_write(&lcd->d5, (nibble >> 1) & 1);
	lcd_pin_write(&lcd->d6, (nibble >> 2) & 1);
	lcd_pin_write(&lcd->d7, (nibble >> 3) & 1);
	lcd_pulse_enable(lcd);	// Hacer un pulso en E 
}

/* ====== API ====== */

// ESCRIBIR COMANDO
void LCD_Command(LCD_4b *lcd, uint8_t cmd){
	lcd_pin_write(&lcd->rs, 0);			// RS = 0 (Comandos)
	lcd_write_nibble(lcd, cmd >> 4);	// Escribir nibble alto 
	lcd_write_nibble(lcd, cmd & 0x0F);	// Escribir nibble bajo
	_delay_ms(2);						// Esperar 2 ms
}

// ESCRIBIR CARACTER EN LCD
void LCD_WriteChar(LCD_4b *lcd, char c){
	lcd_pin_write(&lcd->rs, 1);			// RS = 1 (Datos)
	lcd_write_nibble(lcd, c >> 4);		// Escribir nibble alto
	lcd_write_nibble(lcd, c & 0x0F);	// Escribir nibble bajo
	_delay_us(50);						// Esperar 50 us
}

// ESCRIBIR CADENA DE TEXTO EN LCD
void LCD_WriteString(LCD_4b *lcd, const char *str)
{
	// Recorrer cadena de texto y escribir caracter.
	while (*str) LCD_WriteChar(lcd, *str++);
	// while (*str) recorre la cadena hasta encontrar caracter nulo.
}

//  FIJAR POSICIÓN DE DDRAM
void LCD_SetCursor(LCD_4b *lcd, uint8_t col, uint8_t row)
{
	// Operador ternario (Equivale a una sentencia if else)
	uint8_t address = (row == 0) ? 0x00 : 0x40;		// Primera dirección de cada fila
	LCD_Command(lcd, 0x80 | (address + col));		// Set DDRAM Address
}

// BORRAR EL VISUALIZADOR
void LCD_Clear(LCD_4b *lcd)
{
	LCD_Command(lcd, 0x01);	// Comando 0x01
	_delay_ms(2);
}

// INICIALIZAR LCD
void LCD_Init_4b(LCD_4b *lcd)
{
	/* Configurar pines de salida */
	lcd_pin_output(&lcd->rs);
	lcd_pin_output(&lcd->e);
	lcd_pin_output(&lcd->d4);
	lcd_pin_output(&lcd->d5);
	lcd_pin_output(&lcd->d6);
	lcd_pin_output(&lcd->d7);

	_delay_ms(20);

	/* Secuencia de inicialización */
	lcd_pin_write(&lcd->rs, 0);		// RS = 0

	// SET 8 BIT MODE - 3 veces
	lcd_write_nibble(lcd, 0x03);
	_delay_ms(5);
	lcd_write_nibble(lcd, 0x03);
	_delay_us(150);
	lcd_write_nibble(lcd, 0x03);
	
	// SET 4 BIT MODE
	lcd_write_nibble(lcd, 0x02);

	// CONFIGURACIONES
	LCD_Command(lcd, 0x28); // 4 bits, 2 líneas, 5x8
	LCD_Command(lcd, 0x0C); // Display ON
	LCD_Command(lcd, 0x06); // Entry mode
	LCD_Clear(lcd);
}
