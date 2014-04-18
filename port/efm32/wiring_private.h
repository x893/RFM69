#ifndef WiringPrivate_h
#define WiringPrivate_h

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include <Arduino.h>
#include "em_gpio.h"

#ifdef __cplusplus
extern "C"{
#endif

#define EXTERNAL_INT_0 0
#define EXTERNAL_INT_1 1
#define EXTERNAL_INT_2 2
#define EXTERNAL_INT_3 3
#define EXTERNAL_INT_4 4
#define EXTERNAL_INT_5 5
#define EXTERNAL_INT_6 6
#define EXTERNAL_INT_7 7

typedef void (*voidFuncPtr)(void);

typedef struct GpioPin_s {
	GPIO_Port_TypeDef Port;
	unsigned int Pin;
} GpioPin_t;

#define PIN(pin)	(pin & 0x0F)
#define PORT(pin)	((GPIO_Port_TypeDef)((pin & 0xF0) >> 4))

bool pinToGpio(uint8_t pin, GpioPin_t * gpio);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
