
#include "uart.h"

void UART_init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);	// Habilitar recepción y transmisión
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Configurar 8 bits de datos
}

void UART_sendChar(char data) {
    while (!(UCSR0A & (1 << UDRE0)));		// Esperar a que el registro esté listo
    UDR0 = data;							// Enviar el dato
}

void UART_sendString(const char *str) {
    while (*str) {
        UART_sendChar(*str++);
    }
}
