#ifndef Arduino_h
#define Arduino_h

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <avr/pgmspace.h>

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"{
#endif

#define __extension__

#define interrupts()	__enable_irq()
#define noInterrupts()	__disable_irq()

typedef uint8_t byte;
typedef bool boolean;
typedef uint16_t word;

enum {
	HIGH = 0x1,
	LOW  = 0x0
};

enum {
	INPUT			= 0x0,
	OUTPUT			= 0x1,
	INPUT_PULLUP	= 0x2,
	INPUT_DISABLED	= 0x3
};

enum {
	LSBFIRST	= 0,
	MSBFIRST	= 1
};

enum {
	CHANGE	= 1,
	FALLING	= 2,
	RISING	= 3
};

void init(void);
void setup(void);
void loop(void);

void pinMode(uint8_t, uint8_t);
void digitalWrite(uint8_t, uint8_t);
int digitalRead(uint8_t);

void delay(uint32_t ms);
uint32_t millis(void);

void attachInterrupt(uint8_t, void (*)(void), int mode);
void detachInterrupt(uint8_t);

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
	#include "HardwareSerial.h"
#endif

#include "pins_arduino.h"

#define USART_INIT(baud, mode)
#define USART_DEINIT()
#define USART_WRITE(data)
#define USART_READ()				(0)
#define USART_STATUS(state)			(1)
#define		USART_RX_READY			0x01
#define		USART_TX_COMPLETE		0x02
#define	USART_CONTROL(state)
#define		USART_RX_INT_ENABLE		0x01
#define		USART_RX_INT_DISABLE	0x02
#define		USART_TX_INT_ENABLE		0x04
#define		USART_TX_INT_DISABLE	0x18

#endif
