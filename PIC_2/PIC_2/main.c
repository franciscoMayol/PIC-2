

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include "lcd_i2c.h" 
#include "adc.h"   
#include "uart.h"  
#include "celda_de_carga.h"

#define BAUDRATE 103
#define ON_OFF_1 PD2  // Definir el pin donde está conectado el pulsador
#define LED_VERDE_1 PD4
#define LED_ROJO_1 PD3 
#define BUZZER_1 PD5

#define ON_OFF_2 PD6
#define LED_VERDE_2 PB0
#define LED_ROJO_2 PD7
#define BUZZER_2 PB1



uint32_t tiempo_inactividad = 0;    // Contador para medir inactividad LCD 1
uint32_t tiempo_espera = 60000;

uint32_t tiempo_inactividad2 = 0;    // Contador para medir inactividad LCD 2
uint32_t tiempo_espera2 = 60000;

// Variables para el control del estado del LCD
uint8_t lcd_encendido = 1;   // Estado del LCD: 1 = encendido, 0 = apagado
uint8_t pulsador_presionado = 0;  // Estado del pulsador para manejar el debounce

uint8_t lcd_encendido2 = 1;   // Estado del LCD: 1 = encendido, 0 = apagado
uint8_t pulsador_presionado2 = 0;

void alarmaBuzzer(int buzzer);
void confirmacionBuzzer(int buzzer);
void OnOff1(void);
void OnOff2(void);

void variarLCD1();
void variarLCD2();

void pwm_init_LED1(void);
void pwm_stop_LED1(void);

int main(void){
	
	 long offset1 = 0, offset2 = 0;
	 float scale1 = -219.0, scale2 = -219.0;
	
	 HX711_init();       // Inicializa las celdas de carga
	 
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
	LCD_init(LCD_ADDR2);          // Inicializar el segundo LCD con la dirección 0x26
	_delay_ms(100);
	LCD_backlight_on();
	_delay_ms(100);

	DDRD &= ~((1 << ON_OFF_1) | (1 << ON_OFF_2));
	PORTD |= (1 << ON_OFF_1) | (1 << ON_OFF_2);
	
	DDRD |= (1 << LED_ROJO_2) | (1 << LED_ROJO_1) | (1 << BUZZER_1) | (1 << LED_VERDE_1);
	DDRB |= (1 << BUZZER_2) | (1 << LED_VERDE_2);
	
	UART_sendString("Balanza con dos celdas de carga\r\n");
	// Realiza la tara inicial
	UART_sendString("Tara celda 1...\r\n");
	tare(DT1, SCK1, &offset1);
	UART_sendString("Tara celda 2...\r\n");
	tare(DT2, SCK2, &offset2);
	
	

    while(1)
    {
		
		OnOff1();
	
		// Si el LCD está encendido, actualizar el valor mostrado
		if (lcd_encendido == 1) {
			variarLCD1();   // Actualizar el valor en la pantalla si está encendida
			
			
			tiempo_inactividad +=232;
			
			if (tiempo_inactividad >= tiempo_espera) {
				switch_lcd(LCD_ADDR1);
				LCD_backlight_off();     // Apagar la retroiluminación
				lcd_encendido = 0;       // Marcar el LCD como apagado
			}				
		}
		
		float weight1 = get_units(DT1, SCK1, offset1, scale1);
		char bufferPeso[10];
		char bufferPesoUART[10];
		if(weight1 < 0){
			float cero = 0;
			sprintf(bufferPeso, "%.0f g", cero);
		}
		else{
			sprintf(bufferPeso, "%.0f g", weight1);
		}
		
		LCD_set_cursor(1, 0);              // Mover el cursor a la segunda línea
		_delay_ms(10);
		LCD_write_string("                "); // Limpiar la pantalla antes de mostrar el nuevo valor
		
		_delay_ms(2); // Esperar para que la limpieza se complete
		
		LCD_set_cursor(1, 0);              // Mover el cursor a la segunda línea
		_delay_ms(10);
		LCD_write_string("Peso: ");
		_delay_ms(10);
		LCD_write_string(bufferPeso);          // Mostrar la cantidad de productos
		
		if(weight1 < 0){
			float cero = 0;
			sprintf(bufferPesoUART, "Peso 1 %.0f g \n", cero);
		}
		else{
			sprintf(bufferPesoUART, "Peso 1 %.0f g \n", weight1);
		}
		UART_sendString(bufferPesoUART);
		

		_delay_ms(200);  // Pequeña espera para evitar reacciones rápidas (debounce y tiempo de refresco)
		
		
		
		
		
		
		
		
		OnOff2();
		
		
		// Si el LCD está encendido, actualizar el valor mostrado
		if (lcd_encendido2 == 1) {
			variarLCD2();   // Actualizar el valor en la pantalla si está encendida
			

			tiempo_inactividad2 +=232;
			if (tiempo_inactividad2 >= tiempo_espera2) {
				switch_lcd(LCD_ADDR2);
				LCD_backlight_off();     // Apagar la retroiluminación
				lcd_encendido2 = 0;       // Marcar el LCD como apagado
			}
		}
		
		float weight2 = get_units(DT2, SCK2, offset2, scale2);
		char bufferPeso2[10];
		char bufferPesoUART2[10];
		if(weight2 < 0){
			float cero = 0;
			sprintf(bufferPeso2, "%.0f g", cero);
		}
		else{
			sprintf(bufferPeso2, "%.0f g", weight2);
		}
		
		LCD_set_cursor(1, 0);              // Mover el cursor a la segunda línea
		_delay_ms(10);
		LCD_write_string("                "); // Limpiar la pantalla antes de mostrar el nuevo valor
		
		_delay_ms(2); // Esperar para que la limpieza se complete
		
		LCD_set_cursor(1, 0);              // Mover el cursor a la segunda línea
		_delay_ms(10);
		LCD_write_string("Peso: ");
		_delay_ms(10);
		LCD_write_string(bufferPeso2);          // Mostrar la cantidad de productos
		
		if(weight2 < 0){
			float cero = 0;
			sprintf(bufferPesoUART2, "Peso 2 %.0f g \n", cero);
		}
		else{
			sprintf(bufferPesoUART2, "Peso 2 %.0f g \n", weight2);
		}
		UART_sendString(bufferPesoUART2);
	
		_delay_ms(200);  // Pequeña espera para evitar reacciones rápidas (debounce y tiempo de refresco)
         
    }
}


void variarLCD1() {
	static uint16_t productos_anterior = 0xFFFF;  // Variable estática para almacenar el valor previo
	uint16_t adc_value = 0; // Leer el valor actual del ADC
	uint16_t promedio = 0;        
	
	for(int i=0; i<10; i++){
		adc_value = readADC(0);
		promedio += adc_value; 
		_delay_us(10);
	}    

	// Mapear los valores ADC (0-1023) a productos (0-500)
	uint16_t productos = (promedio/10) / (1024 / 100);
	
	if(productos == 0){
		PORTD |= (1 << LED_ROJO_1);
		alarmaBuzzer(1);
	}
	else{
		PORTD &= ~((1 << LED_ROJO_1) | (1 << BUZZER_1));
	}

	// Solo actualizar la pantalla si la cantidad de productos ha cambiado
	if (productos != productos_anterior) {
		// Crear un buffer para convertir la cantidad de productos a cadena
		char buffer[10];
		char bufferUART[10];
		
		sprintf(buffer, "%i", productos);  // Convertir el número de productos a cadena

		// Seleccionar el primer LCD
		switch_lcd(LCD_ADDR1);
		LCD_set_cursor(0, 0);              // Mover el cursor a la primera línea y primera posición
		_delay_ms(10);
		LCD_write_string("                "); // Limpiar la pantalla antes de mostrar el nuevo valor

		
		
		_delay_ms(2); // Esperar para que la limpieza se complete

		// Mostrar la cantidad de productos en el LCD
		LCD_set_cursor(0, 0);              // Mover el cursor a la primera línea y primera posición
		_delay_ms(10);
		LCD_write_string("Stock: ");   
		_delay_ms(10);
		LCD_write_string(buffer);          // Mostrar la cantidad de productos
		
		sprintf(bufferUART, "Stock 1 %i \n", productos);
		UART_sendString(bufferUART);
		
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
			PORTD |= (1 << LED_VERDE_1);
			_delay_ms(2000);
			PORTD &= ~(1 << LED_VERDE_1);
		}
	}
}


void variarLCD2() {
	static uint16_t productos_anterior = 0xFFFF;  // Variable estática para almacenar el valor previo
	uint16_t adc_value = readADC(1);              // Leer el valor actual del ADC

	// Mapear los valores ADC (0-1023) a productos (0-500)
	uint16_t productos = adc_value / (1024 / 100);
	
	if(productos == 0){
		PORTD |= (1 << LED_ROJO_2);
		alarmaBuzzer(2);
	}
	else{
		PORTD &= ~(1 << LED_ROJO_2);
		PORTB &= ~(1 << BUZZER_2);
	}

	// Solo actualizar la pantalla si la cantidad de productos ha cambiado
	if (productos != productos_anterior) {
		// Crear un buffer para convertir la cantidad de productos a cadena
		char buffer[10];
		char bufferUART[10];
		
		sprintf(buffer, "%i", productos);  // Convertir el número de productos a cadena

		// Seleccionar el primer LCD
		switch_lcd(LCD_ADDR2);
		LCD_set_cursor(0, 0);              // Mover el cursor a la primera línea y primera posición
		_delay_ms(10);
		LCD_write_string("                "); // Limpiar la pantalla antes de mostrar el nuevo valor

		
		
		_delay_ms(2); // Esperar para que la limpieza se complete

		// Mostrar la cantidad de productos en el LCD
		LCD_set_cursor(0, 0);              // Mover el cursor a la primera línea y primera posición
		_delay_ms(10);
		LCD_write_string("Stock: ");
		_delay_ms(10);
		LCD_write_string(buffer);          // Mostrar la cantidad de productos
		
		sprintf(bufferUART, "Stock 2 %i \n", productos);
		UART_sendString(bufferUART);
		
		
		PORTB |= (1 << LED_VERDE_2);
		_delay_ms(50);
		PORTB &= ~(1 << LED_VERDE_2);
		_delay_ms(50);
		
		
		// Actualizar el valor anterior con el nuevo valor
		productos_anterior = productos;
		tiempo_inactividad2 = 0;
	}
	else{
		if(tiempo_inactividad2 >= 5000 && tiempo_inactividad2 <= 5100){
			confirmacionBuzzer(2);
			PORTB |= (1 << LED_VERDE_2);
			_delay_ms(2000);
			PORTB &= ~(1 << LED_VERDE_2);
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
		PORTB |= (1 << BUZZER_2); //Poner buzzer 2
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
		PORTB |= (1 << BUZZER_2); //Poner buzzer 2
		_delay_ms(50);
		PORTB &= ~(1 << BUZZER_2);
		_delay_ms(50);
		PORTB |= (1 << BUZZER_2); //Poner buzzer 2
		_delay_ms(50);
		PORTB &= ~(1 << BUZZER_2);
		_delay_ms(50);
	}
	
}

void OnOff1(void){
	
	switch_lcd(LCD_ADDR1);
	if (!(PIND & (1 << ON_OFF_1))) {
		if (!pulsador_presionado) {
			// Si el pulsador estaba sin presionar y ahora está presionado, cambiar el estado del LCD
			pulsador_presionado = 1;  // Marcar que el pulsador fue presionado

			// Alternar el estado del LCD
			lcd_encendido = !lcd_encendido;   // Alternar el estado: de encendido a apagado, o viceversa

			if (lcd_encendido) {
				// Si el LCD está encendido, encender la retroiluminación
				LCD_backlight_on();
				tiempo_inactividad = 0;
				} else {
				// Si el LCD está apagado, apagar la retroiluminación
				LCD_backlight_off();
			}
		}
		} else {
		// Si el pulsador no está presionado, resetear la variable para el próximo ciclo
		pulsador_presionado = 0;
	}
	
}

void OnOff2(void){
	
	switch_lcd(LCD_ADDR2);
	if (!(PIND & (1 << ON_OFF_2))) {
		if (!pulsador_presionado2) {
			// Si el pulsador estaba sin presionar y ahora está presionado, cambiar el estado del LCD
			pulsador_presionado2 = 1;  // Marcar que el pulsador fue presionado

			// Alternar el estado del LCD
			lcd_encendido2 = !lcd_encendido2;   // Alternar el estado: de encendido a apagado, o viceversa

			if (lcd_encendido2) {
				// Si el LCD está encendido, encender la retroiluminación
				LCD_backlight_on();
				tiempo_inactividad2 = 0;
				} else {
				// Si el LCD está apagado, apagar la retroiluminación
				LCD_backlight_off();
			}
		}
		} else {
		// Si el pulsador no está presionado, resetear la variable para el próximo ciclo
		pulsador_presionado2 = 0;
	}
	
}

