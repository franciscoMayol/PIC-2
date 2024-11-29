#include "celda_de_carga.h"
#include <util/delay.h>

void HX711_init() {
    // Configura los pines de ambas celdas
    DDRB &= ~((1 << DT1) | (1 << DT2));  // DT1 y DT2 como entrada
    DDRB |= (1 << SCK1) | (1 << SCK2);  // SCK1 y SCK2 como salida
    PORTB &= ~((1 << SCK1) | (1 << SCK2)); // Asegura SCK1 y SCK2 en bajo inicialmente
}

void HX711_power_down(uint8_t sck_pin) {
    PORTB |= (1 << sck_pin);  // Lleva SCK a alto por más de 60 µs
    _delay_us(70);
}

void HX711_power_up(uint8_t sck_pin) {
    PORTB &= ~(1 << sck_pin);  // Lleva SCK a bajo para despertar
    _delay_us(1);
}

long HX711_read(uint8_t dt_pin, uint8_t sck_pin) {
    long count = 0;
    while (PINB & (1 << dt_pin));  // Espera hasta que DT esté en bajo

    for (uint8_t i = 0; i < 24; i++) {
        PORTB |= (1 << sck_pin);  // Pulso SCK a alto
        _delay_us(1);
        count = count << 1;       // Desplaza el valor
        if (PINB & (1 << dt_pin)) {
            count++;
        }
        PORTB &= ~(1 << sck_pin); // Pulso SCK a bajo
        _delay_us(1);
    }

    // Pulso adicional para ajustar ganancia (128)
    PORTB |= (1 << sck_pin);
    _delay_us(1);
    PORTB &= ~(1 << sck_pin);
    _delay_us(1);

    // Convierte a valor firmado
    count ^= 0x800000;

    return count;
}

void tare(uint8_t dt_pin, uint8_t sck_pin, long *offset) {
    long sum = 0;
    for (uint8_t i = 0; i < 10; i++) {
        sum += HX711_read(dt_pin, sck_pin);
    }
    *offset = sum / 10;
}

float get_units(uint8_t dt_pin, uint8_t sck_pin, long offset, float scale) {
    long reading = HX711_read(dt_pin, sck_pin);
    return ((reading - offset) / scale);
}
