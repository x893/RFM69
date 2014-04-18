#include <stdio.h>
#include <stdarg.h>

#include <Arduino.h>

#include "em_device.h"
#include "em_cmu.h"
#include "em_chip.h"
#include "em_gpio.h"

#include "em_usb.h"
#include "cdc.h"
#include "usbio.h"
#include "wiring_private.h"

#if defined( USB_PRESENT ) && ( USB_COUNT == 1 )
#include "descriptors.h"
#endif

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
	BSP_LedsInit();
	BSP_LedSet(1);

	USBTIMER_Init();
	USBD_Init( &USBD_Init_Config );       /* Start USB CDC functionality  */
	BSP_LedClear(1);
}

const uint8_t pin2Pin[] = {
	PIN_UNUSED,	PIN_UNUSED,	PIN(PC0),	PIN_UNUSED,	//  0 -  3
	PIN_UNUSED,	PIN_UNUSED,	PIN_UNUSED,	PIN_UNUSED,	//  4 -  7
	PIN_UNUSED,	PIN_UNUSED,	PIN(PD3), 	PIN(PD0),	//  8 - 11
	PIN(PD1),	PIN(PD2),							// 12 - 13
	PIN(PB9),										// 14 - Button PB0
	PIN(PB10),										// 15 - Button PB1
	PIN(PE2),										// 16 - LED0
	PIN(PE3),										// 17 - LED1
};
const GPIO_Port_TypeDef pin2Port [] = {
	gpioPortA,	gpioPortA,	PORT(PC0),	gpioPortA,	//  0 -  3
	gpioPortA,	gpioPortA,	gpioPortA,	gpioPortA,	//  4 -  7
	gpioPortA,	gpioPortA,	PORT(PD3),	PORT(PD0),	//  8 - 11
	PORT(PD1),	PORT(PD2),							// 12 - 13
	PORT(PB9),	PORT(PB10),		// 14-15 Buttons
	PORT(PE2),	PORT(PE3)		// 16-17 LEDs
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
