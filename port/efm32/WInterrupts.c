#include <stdio.h>
#include <stdbool.h>

#include "wiring_private.h"

const uint8_t irq2Pin[] = {
	2,	// INT0 = D2 (PC0)
	14,	// INT1 = 14 (PB9)
	15	// INT2 = 15 (PB10)
};

voidFuncPtr intFunc[sizeof(irq2Pin)];

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode)
{
	bool rising;
	bool falling;
	GpioPin_t gpio;
	uint8_t pin;

	if (interruptNum < sizeof(irq2Pin))
	{
		pin = irq2Pin[interruptNum];
		if (pinToGpio(pin, &gpio))
		{
			rising = falling = false;
			if( mode == RISING )
				rising = true;
			else if( mode == FALLING )
				falling = true;
			else if( mode == CHANGE )
				falling = rising = true;
			else
				return;

			intFunc[interruptNum] = userFunc;
			pinMode(pin, INPUT_PULLUP);
			GPIO_IntConfig(gpio.Port, gpio.Pin, rising, falling, true);
			GPIO_IntClear(1 << gpio.Pin);
			NVIC_EnableIRQ(GPIO_ODD_IRQn);
			NVIC_EnableIRQ(GPIO_EVEN_IRQn);
		}
	}
}

void detachInterrupt(uint8_t interruptNum)
{
	if (interruptNum < sizeof(irq2Pin))
	{
		intFunc[interruptNum] = NULL;

		uint8_t pin = irq2Pin[interruptNum];
		GpioPin_t gpio;

		if (pinToGpio(pin, &gpio))
		{
			GPIO_IntConfig(gpio.Port, gpio.Pin, false, false, false);
		}
	}
}

void GPIO_IRQHandler(void)
{
	register voidFuncPtr * handler = &intFunc[0];
	register uint16_t flags = GPIO_IntGetEnabled();
	register int idx;
	GpioPin_t gpio;

	for (idx = 0; idx < sizeof(irq2Pin); idx++)
	{
		if (pinToGpio(irq2Pin[idx], &gpio))
		{
			if (*handler != NULL && (flags & (1 << gpio.Pin)) != 0)
				(*handler)();
		}
		handler++;
	}
	GPIO_IntClear(flags);
}

void GPIO_EVEN_IRQHandler(void)
{
	GPIO_IRQHandler();
}

void GPIO_ODD_IRQHandler(void)
{
	GPIO_IRQHandler();
}
