#include <Arduino.h>

int main(void)
{
	init();

	while (!Serial.DTR())
	{
		delay(250);
		BSP_LedToggle(1);
	}
	BSP_LedClear(1);

	// setup();

	for (;;)
	{
		// loop();
		if (serialEventRun) serialEventRun();
	}
}
