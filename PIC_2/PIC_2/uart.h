
#ifndef UART_H
#define UART_H

#include <avr/io.h>

// Inicialización de UART
void UART_init(unsigned int ubrr);

// Enviar un carácter por UART
void UART_sendChar(char data);

// Enviar una cadena de caracteres por UART
void UART_sendString(const char *str);

#endif
