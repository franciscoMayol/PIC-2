
#ifndef ADC_H
#define ADC_H

#include <avr/io.h>

// Inicialización del ADC
void setupADC(void);

// Leer valor del ADC
uint16_t readADC(uint16_t channel);

#endif
