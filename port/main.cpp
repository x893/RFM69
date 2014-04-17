#include <Arduino.h>

int main(void)
{
	init();

	Serial.begin(115200);
	while (!Serial.available())
	{
		delay(1000);
		Serial.println("Press a key to start");
	}
	Serial.println("setup() ... ");

	// setup();

	Serial.println("OK");
	Serial.println("loop() TTY echo");
	for (;;)
	{
		while (Serial.available())
		{
			Serial.write(Serial.read());
		}
		// loop();

		if (serialEventRun) serialEventRun();
	}
}
