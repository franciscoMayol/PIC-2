
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "HX711.h"

// Configuración de UART
void UART_init(uint16_t baud) {
	uint16_t UBRR_value = (F_CPU / (16UL * baud)) - 1;
	UBRR0H = (uint8_t)(UBRR_value >> 8);
	UBRR0L = (uint8_t)UBRR_value;
	UCSR0B = (1 << TXEN0);       // Habilita la transmisión
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 bits de datos
}

void UART_send_char(char data) {
	while (!(UCSR0A & (1 << UDRE0)));  // Espera a que el registro esté libre
	UDR0 = data;  // Envía el dato
}

void UART_send_string(const char* str) {
	while (*str) {
		UART_send_char(*str++);
	}
}

int main(void) {
	UART_init(9600);         // Inicializa UART a 9600 baudios
	HX711_init(128);         // Inicializa HX711 en el canal A con ganancia 128
	
	_delay_ms(1000);         // Tiempo de estabilización
	UART_send_string("Tare...\n");
	HX711_tare(10);          // Realiza la tara con 10 lecturas promedio
	HX711_set_scale(-1.0);    // Ajusta el valor de escala aquí según tu calibración

	while (1) {
		float weight = HX711_get_units(10);  // Lee el peso promedio
		char buffer[20];
		snprintf(buffer, sizeof(buffer), "Peso: %.2f\n", weight);
		UART_send_string(buffer);
		_delay_ms(1000);     // Espera 1 segundo antes de la siguiente lectura
	}
}
