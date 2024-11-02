#define F_CPU 16000000UL
#include "lcd_i2c.h"
#include <util/delay.h>

#define F_SCL 100000UL // Frecuencia del I2C

// Variables globales
uint8_t backlight_state = LCD_BACKLIGHT;
uint8_t current_lcd_addr = LCD_ADDR1;

// Función para inicializar el I2C
void I2C_init(void) {
    TWSR = 0x00; // Sin prescaler
    TWBR = ((F_CPU / F_SCL) - 16) / 2;
    TWCR = (1 << TWEN); // Habilitar TWI
}

// Función para iniciar la transmisión I2C
void I2C_start(void) {
    TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT); // Generar condición de start
    while (!(TWCR & (1 << TWINT))); // Esperar a que se complete la condición de start
}

// Función para detener la transmisión I2C
void I2C_stop(void) {
    TWCR = (1 << TWSTO) | (1 << TWEN) | (1 << TWINT); // Generar condición de stop
}

// Función para escribir datos en el bus I2C
void I2C_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWEN) | (1 << TWINT); // Iniciar la transmisión de datos
    while (!(TWCR & (1 << TWINT))); // Esperar a que se complete la transmisión
}

// Función para inicializar el LCD con tiempos de espera aumentados
void LCD_init(uint8_t addr) {
    current_lcd_addr = addr; // Establecer dirección del LCD

    _delay_ms(100); // Esperar a que el LCD arranque
    LCD_send(0x30, LCD_COMMAND); // Inicialización en modo de 8 bits
    _delay_ms(10);
    LCD_send(0x30, LCD_COMMAND);
    _delay_ms(10);
    LCD_send(0x30, LCD_COMMAND);
    _delay_ms(10);
    LCD_send(0x20, LCD_COMMAND); // Cambiar a modo de 4 bits
    _delay_ms(10);

    // Configuración del LCD: 2 líneas, 5x8 puntos
    LCD_command(0x28);
    _delay_ms(5);
    // Encender pantalla, cursor apagado
    LCD_command(0x0C);
    _delay_ms(5);
    // Borrar pantalla
    LCD_command(0x01);
    _delay_ms(10);
    // Configurar modo de entrada: incrementar dirección y sin desplazamiento
    LCD_command(0x06);
    _delay_ms(5);
}

// Función para enviar datos al LCD
void LCD_send(uint8_t data, uint8_t mode) {
    uint8_t high_nibble = (data & 0xF0) | backlight_state | mode;
    uint8_t low_nibble = ((data << 4) & 0xF0) | backlight_state | mode;

    I2C_start();
    I2C_write(current_lcd_addr << 1); // Enviar dirección del esclavo

    // Enviar nibble alto
    I2C_write(high_nibble | 0x04); // Enable alto
    I2C_write(high_nibble);        // Enable bajo

    // Enviar nibble bajo
    I2C_write(low_nibble | 0x04); // Enable alto
    I2C_write(low_nibble);        // Enable bajo

    I2C_stop();
}

// Función para enviar un comando al LCD
void LCD_command(uint8_t cmd) {
    LCD_send(cmd, LCD_COMMAND);
}

// Función para escribir un carácter en el LCD
void LCD_write(char data) {
    LCD_send(data, LCD_DATA);
}

// Función para escribir una cadena de caracteres en el LCD
void LCD_write_string(const char *str) {
    while (*str) {
        LCD_write(*str++);
    }
}

// Función para mover el cursor a una posición específica (fila y columna)
void LCD_set_cursor(uint8_t row, uint8_t col) {
    uint8_t address;

    // Direcciones de memoria para las filas 0 y 1
    if (row == 0) {
        address = 0x00 + col; // Primera línea
    } else {
        address = 0x40 + col; // Segunda línea
    }

    LCD_command(0x80 | address); // Comando para mover el cursor
}

// Función para limpiar la segunda fila del LCD
void LCD_clear_row(uint8_t row) {
    LCD_set_cursor(row, 0);
    LCD_write_string("                ");  // Escribe 16 espacios para limpiar la fila
}

// Función para encender la retroiluminación del LCD
void LCD_backlight_on(void) {
    backlight_state = LCD_BACKLIGHT; // Actualizar el estado de la retroiluminación
    LCD_command(0x00); // Refrescar la pantalla para aplicar el estado
}

// Función para apagar la retroiluminación del LCD
void LCD_backlight_off(void) {
    backlight_state = LCD_NOBACKLIGHT; // Actualizar el estado de la retroiluminación
    LCD_command(0x00); // Refrescar la pantalla para aplicar el estado
}

// Función para cambiar la dirección del LCD activo
void switch_lcd(uint8_t addr) {
    current_lcd_addr = addr; // Cambiar a la dirección del LCD deseado
}



