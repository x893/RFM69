#include <stdio.h>

#include <Arduino.h>
#include "wiring_private.h"
#include "HardwareSerial.h"
#include "usbio.h"

#ifdef __cplusplus
extern "C" {
#endif
	extern bool CDC_Configured;
	void USBTIMER_DelayMs(uint32_t msec);
	void USBD_Disconnect(void);
#ifdef __cplusplus
}
#endif

#define SERIAL_BUFFER_SIZE 64
struct ring_buffer
{
	unsigned char buffer[SERIAL_BUFFER_SIZE];
	volatile unsigned int head;
	volatile unsigned int tail;
};

ring_buffer rx_buffer = { { 0 }, 0, 0};

void storeRxBuffer(void * dst, uint8_t * src, uint32_t xferred, uint32_t remaining)
{
	while (xferred != 0)
	{
		ring_buffer * buffer = (ring_buffer *)dst;
		uint8_t c = *src++;
		int next = (unsigned int)(buffer->head + 1) % SERIAL_BUFFER_SIZE;
		if (next != buffer->tail)
		{
			buffer->buffer[buffer->head] = c;
			buffer->head = next;
		}
		--xferred;
	}
}

void serialEvent() __attribute__((weak));
void serialEvent() { }

void serialEventRun(void)
{
	if (Serial.available()) serialEvent();
}

void HardwareSerial::checkRx()
{
	if (!USB_rxStatusOK())
	{
		USB_rxAccept(storeRxBuffer, _rx_buffer);
	}
}

// Constructors ////////////////////////////////////////////////////////////////

HardwareSerial::HardwareSerial(ring_buffer *rx_buffer)
{
	_rx_buffer = rx_buffer;
}

// Public Methods //////////////////////////////////////////////////////////////

void HardwareSerial::begin(uint32_t baud)
{
	begin(baud, 0);
}

void HardwareSerial::begin(uint32_t baud, byte config)
{
	/* Wait for USB connection */
	while (!CDC_Configured)
	{
		USBTIMER_DelayMs( 100 );
	}
	checkRx();
}

void HardwareSerial::end()
{
	/* Allow time to do a disconnect in a terminal program. */
	USBTIMER_DelayMs( 5000 );

	USBD_Disconnect();
	/*
	 * Stay disconnected long enough to let host OS tear down the
	 * USB CDC driver.
	 */
	USBTIMER_DelayMs( 2000 );
	// clear any received data
	_rx_buffer->head = _rx_buffer->tail;
}

int HardwareSerial::available(void)
{
	checkRx();
	return (unsigned int)(SERIAL_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) % SERIAL_BUFFER_SIZE;
}

int HardwareSerial::peek(void)
{
	checkRx();
	if (_rx_buffer->head == _rx_buffer->tail)
		return -1;
	else
		return _rx_buffer->buffer[_rx_buffer->tail];
}

int HardwareSerial::read(void)
{
	checkRx();
	// if the head isn't ahead of the tail, we don't have any characters
	if (_rx_buffer->head == _rx_buffer->tail)
		return -1;
	else
	{
		unsigned char c = _rx_buffer->buffer[_rx_buffer->tail];
		_rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % SERIAL_BUFFER_SIZE;
		return c;
	}
}

void HardwareSerial::flush()
{
}

size_t HardwareSerial::write(uint8_t c)
{
	USB_txByte( c );
	return 1;
}

HardwareSerial::operator bool()
{
	return true;
}

void HardwareSerial::puts(const char *src)
{
	USB_txString(src);
}

void HardwareSerial::puts(const uint8_t *src, int length)
{
	USB_txBytes(src, length);
}

HardwareSerial Serial(&rx_buffer);
