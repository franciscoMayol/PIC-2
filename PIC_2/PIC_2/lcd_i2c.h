
#ifndef LCD_I2C_H
#define LCD_I2C_H

#include <avr/io.h>

// Definir direcciones de los LCDs
#define LCD_ADDR1 0x27
#define LCD_ADDR2 0x23

// Definir constantes
#define LCD_COMMAND 0
#define LCD_DATA 1
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

// Funciones de inicialización
void I2C_init(void);
void LCD_init(uint8_t addr);
void LCD_backlight_on(void);
void LCD_backlight_off(void);



// Funciones para enviar datos
void LCD_send(uint8_t data, uint8_t mode);
void LCD_command(uint8_t cmd);
void LCD_write(char data);
void LCD_write_string(const char *str);
void LCD_set_cursor(uint8_t row, uint8_t col);

void limpiarPantalla(void);

// Función para cambiar entre LCDs
void switch_lcd(uint8_t addr);

#endif
