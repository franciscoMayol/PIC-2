/*
 * main.c
 *
 * Created: 10/24/2024 2:33:52 PM
 *  Author: MSI
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include "lcd_i2c.h" 
#include "adc.h"   
#include "uart.h"  

#define BAUDRATE 103
#define ON_OFF_1 PD2  
#define LED_VERDE_1 PD3
#define LED_ROJO_1 PD4
#define BUZZER_1 PD5

#define ON_OFF_2 PD6
#define LED_VERDE_2 PD7
#define LED_ROJO_2 PB0
#define BUZZER_2 PB1



uint32_t tiempo_inactividad = 0;    // Contador para medir inactividad LCD 1
uint32_t tiempo_espera = 30000;

uint32_t tiempo_inactividad2 = 0;    // Contador para medir inactividad LCD 2
uint32_t tiempo_espera2 = 30000;

// Variables para el control del estado del LCD
uint8_t lcd_encendido = 1;         // Estado del LCD: 1 = encendido, 0 = apagado
uint8_t pulsador_presionado = 0;   // Estado del pulsador para manejar el debounce

uint8_t lcd_encendido2 = 1;  
uint8_t pulsador_presionado2 = 0;

void alarmaBuzzer(int buzzer);
void confirmacionBuzzer(int buzzer);
void OnOff1(void);
void OnOff2(void);

void variarLCD1(void);
void variarLCD2(void);

void pwm_init_LED1(void);
void pwm_stop_LED1(void);

int main(void){
	
	 // Inicializar el ADC y la UART
	 setupADC();
	 UART_init(BAUDRATE);
	 
	 
	// Inicializar el I2C y los LCDs
	I2C_init();                   // Inicializar el bus I2C
	_delay_ms(100);
	LCD_init(LCD_ADDR1);          // Inicializar el primer LCD con la dirección 0x27
	_delay_ms(100);
	LCD_backlight_on();
	_delay_ms(100);
	LCD_init(LCD_ADDR2);          // Inicializar el segundo LCD con la dirección 0x23
	_delay_ms(100);
	LCD_backlight_on();
	_delay_ms(100);

	DDRD &= ~((1 << ON_OFF_1) | (1 << ON_OFF_2));
	PORTD = (1 << ON_OFF_1) | (1 << ON_OFF_2);
	
	DDRD |= (1 << LED_ROJO_1) | (1 << LED_VERDE_1) | (1 << BUZZER_1) | (1 << LED_VERDE_2);
	DDRB |= (1 << LED_ROJO_2) | (1 << BUZZER_2);
	

    while(1)
    {
		OnOff1();
		

		// Si el LCD1 está encendido, actualizar el valor mostrado
		if (lcd_encendido) {
			variarLCD1();							 // Actualizar el valor en la pantalla 1 si está encendida
			tiempo_inactividad +=200;
			if (tiempo_inactividad >= tiempo_espera) {
				switch_lcd(LCD_ADDR1);
				LCD_backlight_off();				 // Apagar la retroiluminación
				lcd_encendido = 0;					 // Marcar el LCD 1 como apagado
			}				
		}

		_delay_ms(200);  
		
		OnOff2();
		
		// Si el LCD2 está encendido, actualizar el valor mostrado
		if (lcd_encendido2) {
			variarLCD2();							// Actualizar el valor en la pantalla 2 si está encendida
			tiempo_inactividad2 +=200;
			if (tiempo_inactividad2 >= tiempo_espera2) {
				switch_lcd(LCD_ADDR2);
				LCD_backlight_off();				// Apagar la retroiluminación
				lcd_encendido2 = 0;					// Marcar el LCD 2 como apagado
			}
		}

		_delay_ms(200);  // Pequeña espera para evitar reacciones rápidas (debounce y tiempo de refresco)
         
    }
}


void variarLCD1(void) {
	static uint16_t productos_anterior = 0xFFFF;    // Variable estática para almacenar el valor previo
	uint16_t adc_value = 0;							// Leer el valor actual del ADC
	uint16_t promedio = 0;        
	
	for(int i=0; i<10; i++){
		adc_value = readADC(0);
		promedio += adc_value; 
		_delay_us(10);
	}    

	// Mapear los valores ADC (0-1023) a productos (0-500)
	uint16_t productos = (promedio/10) / (1024 / 150);
	
	if(productos == 0){
		PORTD |= (1 << LED_ROJO_1);
		alarmaBuzzer(1);
	}
	else{
		PORTD &= ~((1 << LED_ROJO_1) | (1 << BUZZER_1));
	}

	// Solo actualizar la pantalla si la cantidad de productos ha cambiado
	if (productos != productos_anterior) {
		
		char buffer[10];
		sprintf(buffer, "%i", productos);  

		// Seleccionar el primer LCD
		switch_lcd(LCD_ADDR1);
		LCD_set_cursor(0, 0);                 // Mover el cursor a la primera línea y primera posición
		_delay_ms(10);
		LCD_write_string("                "); // Limpiar la pantalla antes de mostrar el nuevo valor

		
		_delay_ms(2); 

		// Mostrar la cantidad de productos en el LCD
		LCD_set_cursor(0, 0);              // Mover el cursor a la primera línea y primera posición
		_delay_ms(10);
		LCD_write_string("Stock: ");   
		_delay_ms(10);
		LCD_write_string(buffer);          // Mostrar la cantidad de productos
		
		
		LCD_set_cursor(1, 0);              // Mover el cursor a la segunda línea y primera posición
		_delay_ms(10);
		LCD_write_string("                "); // Limpiar la pantalla antes de mostrar el nuevo valor
		
		_delay_ms(2); 
		
		LCD_set_cursor(1, 0);              // Mover el cursor a la segunda línea y primera posición
		_delay_ms(10);
		LCD_write_string("Peso: ");
		_delay_ms(10);
		LCD_write_string("hola");          // Mostrar el peso
		
		PORTD |= (1 << LED_VERDE_1);
		_delay_ms(50);
		PORTD &= ~(1 << LED_VERDE_1);
		_delay_ms(50);
		
		
		// Actualizar el valor anterior con el nuevo valor
		productos_anterior = productos;
		tiempo_inactividad = 0;
	}
	else{
		if(tiempo_inactividad >= 5000 && tiempo_inactividad <= 5100){
			confirmacionBuzzer(1);
			PORTD |= (1 << LED_VERDE_1);	// Si pasan aproximadamente 5 segundos sin variar se confimra el valor de stock
			_delay_ms(2000);
			PORTD &= ~(1 << LED_VERDE_1);
		}
	}
}


void variarLCD2(void) {
	static uint16_t productos_anterior = 0xFFFF;	// Variable estática para almacenar el valor previo
	uint16_t adc_value = readADC(1);				// Leer el valor actual del ADC

	// Mapear los valores ADC (0-1023) a productos (0-500)
	uint16_t productos = adc_value / (1024 / 150);
	
	if(productos == 0){
		PORTB |= (1 << LED_ROJO_2);
		alarmaBuzzer(2);
	}
	else{
		PORTB &= ~((1 << LED_ROJO_2) | (1 << BUZZER_2));
	}

	// Solo actualizar la pantalla si la cantidad de productos ha cambiado
	if (productos != productos_anterior) {
		
		char buffer[10];
		sprintf(buffer, "%i", productos);  

		// Seleccionar el segundo LCD
		switch_lcd(LCD_ADDR2);
		LCD_set_cursor(0, 0);                 // Mover el cursor a la primera línea y primera posición
		_delay_ms(10);
		LCD_write_string("                "); // Limpiar la pantalla antes de mostrar el nuevo valor

		
		
		_delay_ms(2); 

		// Mostrar la cantidad de productos en el LCD 2
		LCD_set_cursor(0, 0);              // Mover el cursor a la primera línea y primera posición
		_delay_ms(10);
		LCD_write_string("Stock: ");
		_delay_ms(10);
		LCD_write_string(buffer);          // Mostrar la cantidad de productos
		
		
		LCD_set_cursor(1, 0);              // Mover el cursor a la segunda línea y primera posición
		_delay_ms(10);
		LCD_write_string("                "); // Limpiar la pantalla antes de mostrar el nuevo valor
		
		_delay_ms(2); 
		
		LCD_set_cursor(1, 0);              // Mover el cursor a la segunda línea y primera posición
		_delay_ms(10);
		LCD_write_string("Peso: ");
		_delay_ms(10);
		LCD_write_string("hola");          // Mostrar el peso
		
		PORTD |= (1 << LED_VERDE_2);
		_delay_ms(50);
		PORTD &= ~(1 << LED_VERDE_2);
		_delay_ms(50);
		
		
		// Actualizar el valor anterior con el nuevo valor
		productos_anterior = productos;
		tiempo_inactividad2 = 0;
	}
	else{
		if(tiempo_inactividad2 >= 5000 && tiempo_inactividad2 <= 5100){
			confirmacionBuzzer(2);
			PORTD |= (1 << LED_VERDE_2);	// Si pasan aproximadamente 5 segundos sin variar se confimra el valor de stock
			_delay_ms(2000);
			PORTD &= ~(1 << LED_VERDE_2);
		}
	}
}





void alarmaBuzzer(int buzzer){
	
	if(buzzer == 1){
		PORTD |= (1 << BUZZER_1);
		_delay_ms(50);
		PORTD &= ~(1 << BUZZER_1);
		_delay_ms(50);
	}
	else if (buzzer == 2){
		PORTB |= (1 << BUZZER_2); 
		_delay_ms(50);
		PORTB &= ~(1 << BUZZER_2);
		_delay_ms(50);
	}
	
}

void confirmacionBuzzer(int buzzer){
	
	if(buzzer == 1){
		PORTD |= (1 << BUZZER_1);
		_delay_ms(50);
		PORTD &= ~(1 << BUZZER_1);
		_delay_ms(50);
		PORTD |= (1 << BUZZER_1);
		_delay_ms(50);
		PORTD &= ~(1 << BUZZER_1);
		_delay_ms(50);
	}
	else if (buzzer == 2){
		PORTB |= (1 << BUZZER_2); 
		_delay_ms(50);
		PORTB &= ~(1 << BUZZER_2);
		_delay_ms(50);
		PORTB |= (1 << BUZZER_2); 
		_delay_ms(50);
		PORTB &= ~(1 << BUZZER_2);
		_delay_ms(50);
	}
	
}

void OnOff1(void){
	
	switch_lcd(LCD_ADDR1);
	if (!(PIND & (1 << ON_OFF_1))) {
		if (!pulsador_presionado) {
			// Si el pulsador estaba sin presionar y ahora está presionado, cambiar el estado del LCD 1
			pulsador_presionado = 1;  // Marcar que el pulsador fue presionado

			lcd_encendido = !lcd_encendido;   // Alternar el estado del LCD 1

			if (lcd_encendido) {
				LCD_backlight_on();		// Si el LCD 1 está encendido, encender la retroiluminación
				tiempo_inactividad = 0;
				} else {
				LCD_backlight_off();    // Si el LCD 1 está apagado, apagar la retroiluminación
			}
		}
		} else {
		pulsador_presionado = 0;	// Si el pulsador no está presionado, resetear la variable para el próximo ciclo
	}
	
}

void OnOff2(void){
	
	switch_lcd(LCD_ADDR2);
	if (!(PIND & (1 << ON_OFF_2))) {
		if (!pulsador_presionado2) {
			// Si el pulsador estaba sin presionar y ahora está presionado, cambiar el estado del LCD 2
			pulsador_presionado2 = 1;  // Marcar que el pulsador fue presionado
			
			lcd_encendido2 = !lcd_encendido2;    // Alternar el estado del LCD 2

			if (lcd_encendido2) {
				LCD_backlight_on();			// Si el LCD 2 está encendido, encender la retroiluminación
				tiempo_inactividad2 = 0;
				} else {
				LCD_backlight_off();		// Si el LCD 2 está apagado, apagar la retroiluminación
			}
		}
		} else {
		pulsador_presionado2 = 0;		// Si el pulsador no está presionado, resetear la variable para el próximo ciclo
	}
	
}

