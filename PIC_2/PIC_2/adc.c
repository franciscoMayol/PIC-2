
#include "adc.h"

void setupADC(void) {
    ADMUX |= (1 << REFS0); 
    ADMUX &= ~(1 << ADLAR); 
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1); // Prescaler de 64

    // Habilitar el ADC
    ADCSRA |= (1 << ADEN);
}

uint16_t readADC(uint16_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);  // Selecciona el canal
    ADCSRA |= (1 << ADSC);						// Inicia la conversión

    while (ADCSRA & (1 << ADSC));				// Espera a que termine la conversión
    return ADC;									// Retorna valor digital entre 0 y 1023
}
