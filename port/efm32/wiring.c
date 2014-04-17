#include <stdio.h>
#include <stdarg.h>

#include <Arduino.h>

#include "em_device.h"
#include "em_cmu.h"
#include "em_chip.h"
#include "em_gpio.h"

#include "bsp.h"

#include "em_usb.h"
#include "cdc.h"
#include "usbio.h"

#include "descriptors.h"

/** Version string, used when the user connects */
#define USBCDC_VERSION_STRING "USBCDC 1.01"

volatile uint32_t timer0_millis = 0;
void SysTick_Handler(void)
{
	timer0_millis++;
}

uint32_t millis()
{
	return timer0_millis;
}

void delay(uint32_t ms)
{
	uint32_t start = millis();
	while (ms > 0) {
		if (start != millis()) {
			start = millis();
			ms--;
		}
	}
}

void init()
{
	/* Initialize chip */
	CHIP_Init();

	/* Start HFXO. */
	CMU_OscillatorEnable( cmuOsc_HFXO, true, true );
	CMU_ClockSelectSet( cmuClock_HF, cmuSelect_HFXO );
	/* Enable clock for GPIO module */
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(cmuClock_DMA, true);

	/* Set 1 ms interval */
	if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000))
		while (1)
			;

	BSP_LedsInit();
	BSP_LedSet(1);

	USBTIMER_Init();
	USBD_Init( &initstruct );       /* Start USB CDC functionality  */
	BSP_LedClear(0);
}

typedef struct GpioPin_s {
	GPIO_Port_TypeDef Port;
	unsigned int Pin;
} GpioPin_t;

#define PIN(pin)	(pin & 0x0F)
#define PORT(pin)	((GPIO_Port_TypeDef)((pin & 0xF0) >> 4))

const uint8_t pin2Pin[] = {
	PIN_UNUSED,	PIN_UNUSED,	PIN_UNUSED,	PIN_UNUSED,	//  0 -  3
	PIN_UNUSED,	PIN_UNUSED,	PIN_UNUSED,	PIN_UNUSED,	//  4 -  7
	PIN_UNUSED,	PIN_UNUSED,	PIN(PD3), 	PIN(PD0),	//  8 - 11
	PIN(PD1),	PIN(PD2)							// 12 - 13
};
const GPIO_Port_TypeDef pin2Port [] = {
	gpioPortA,	gpioPortA,	gpioPortA,	gpioPortA,	//  0 -  3
	gpioPortA,	gpioPortA,	gpioPortA,	gpioPortA,	//  4 -  7
	gpioPortA,	gpioPortA,	PORT(PD3),	PORT(PD0),	//  8 - 11
	PORT(PD1),	PORT(PD2)							// 12 - 13
};

bool pinToGpio(uint8_t pin, GpioPin_t * gpio)
{
	if (pin < sizeof(pin2Pin))
	{
		gpio->Pin = pin2Pin[pin];
		if (gpio->Pin != PIN_UNUSED)
		{
			gpio->Port = pin2Port[pin];
			return true;
		}
	}
	return false;
}

void pinMode(uint8_t pin, uint8_t mode)
{
	GpioPin_t gpio;
	if (pinToGpio(pin, &gpio))
	{
		switch (mode)
		{
			case INPUT:
				GPIO_PinModeSet(
					gpio.Port, gpio.Pin,
					gpioModeInput,
					0
				);
				break;
			case OUTPUT:
				GPIO_PinModeSet(
					gpio.Port, gpio.Pin,
					gpioModePushPull,
					GPIO_PinOutGet(gpio.Port, gpio.Pin)
				);
				break;
			case INPUT_PULLUP:
				GPIO_PinModeSet(
					gpio.Port, gpio.Pin,
					gpioModeInputPull,
					1
				);
				break;
			case INPUT_DISABLED:
				GPIO_PinModeSet(
					gpio.Port, gpio.Pin,
					gpioModeDisabled,
					1
				);
				break;
		}
	}
}
void digitalWrite(uint8_t pin, uint8_t value)
{
	GpioPin_t gpio;
	if (pinToGpio(pin, &gpio))
	{
		if (value)
			GPIO_PinOutSet(gpio.Port, gpio.Pin);
		else
			GPIO_PinOutClear(gpio.Port, gpio.Pin);
	}
}

int digitalRead(uint8_t pin)
{
	GpioPin_t gpio;
	if (pinToGpio(pin, &gpio))
	{
		return GPIO_PinInGet(gpio.Port, gpio.Pin);
	}
	return 0;
}
