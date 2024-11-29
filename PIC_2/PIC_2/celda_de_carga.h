#ifndef CELDA_DE_CARGA_H
#define CELDA_DE_CARGA_H

#include <avr/io.h>

// Pines configurados para las celdas de carga
#define DT1 PB4   // DT de la celda 1 en PB4
#define SCK1 PB2  // SCK de la celda 1 en PB2
#define DT2 PB5   // DT de la celda 2 en PB5
#define SCK2 PB3  // SCK de la celda 2 en PB3

// Declaración de funciones
void HX711_init();
void HX711_power_down(uint8_t sck_pin);
void HX711_power_up(uint8_t sck_pin);
long HX711_read(uint8_t dt_pin, uint8_t sck_pin);
void tare(uint8_t dt_pin, uint8_t sck_pin, long *offset);
float get_units(uint8_t dt_pin, uint8_t sck_pin, long offset, float scale);

#endif
