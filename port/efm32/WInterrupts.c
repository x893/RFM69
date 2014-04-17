#include <inttypes.h>
#include <stdio.h>

#include "wiring_private.h"

static volatile voidFuncPtr intFunc[EXTERNAL_NUM_INTERRUPTS];
// volatile static voidFuncPtr twiIntFunc;

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode)
{
	if(interruptNum < EXTERNAL_NUM_INTERRUPTS)
	{
		intFunc[interruptNum] = userFunc;

		// Configure the interrupt mode (trigger on low input, any change, rising
		// edge, or falling edge).  The mode constants were chosen to correspond
		// to the configuration bits in the hardware register, so we simply shift
		// the mode into place.

		// Enable the interrupt.
      
		switch (interruptNum)
		{
			// I hate doing this, but the register assignment differs between the 1280/2560
			// and the 32U4.  Since avrlib defines registers PCMSK1 and PCMSK2 that aren't 
			// even present on the 32U4 this is the only way to distinguish between them.
			case 0:
				break;
		}
	}
}

void detachInterrupt(uint8_t interruptNum)
{
	if(interruptNum < EXTERNAL_NUM_INTERRUPTS)
	{
		// Disable the interrupt.
		switch (interruptNum)
		{
			case 0:
				break;
		}
		intFunc[interruptNum] = 0;
	}
}

void INT0_IRQ_HANDLER(void)
{
	if (intFunc[EXTERNAL_INT_0])
		intFunc[EXTERNAL_INT_0]();
}

