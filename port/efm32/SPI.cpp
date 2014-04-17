#include <Arduino.h>
#include "SPI.h"

SPIClass SPI(0);

const SPI_t spi2SPI[] = {
	{ USART1, cmuClock_USART1, USART_ROUTE_LOCATION_LOC1, MOSI, MISO, SCK, SS, UART1_RX_IRQn, UART1_TX_IRQn }
};
SPIClass::SPIClass(uint8_t spi_number)
{
	if (spi_number < 1)
	{
		const SPI_t * spi = &spi2SPI[spi_number];

		spi_init.clock				= spi->clock;
		spi_init.usart				= spi->usart;

		spi_init.init.enable		= usartEnable;
		spi_init.init.baudrate		= 400000;
		spi_init.init.databits		= usartDatabits8;
		spi_init.init.msbf			= false;
		spi_init.init.master		= 1;
		spi_init.init.clockMode		= usartClockMode0;
		spi_init.init.prsRxEnable	= 0;
		spi_init.init.autoTx		= 0;

		spi_init.mosi	= spi->mosi;
		spi_init.miso	= spi->miso;
		spi_init.sck	= spi->sck;
		spi_init.ss		= spi->ss;
	}
}

void SPIClass::reinit()
{
	USART_Reset(spi_init.usart);
	USART_InitSync(spi_init.usart, &(spi_init.init));
	spi_init.usart->ROUTE =	spi_init.route |
							USART_ROUTE_TXPEN |
							USART_ROUTE_RXPEN |
							USART_ROUTE_CLKPEN;
}

void SPIClass::begin()
{
	digitalWrite(spi_init.ss, HIGH);
	pinMode(spi_init.ss, OUTPUT);
	pinMode(spi_init.sck, OUTPUT);
	pinMode(spi_init.mosi, OUTPUT);
	pinMode(spi_init.miso, INPUT_PULLUP);

	CMU_ClockEnable(spi_init.clock, true);

	reinit();
}

void SPIClass::end()
{
	spi_init.usart->ROUTE =	0;
	USART_Reset(spi_init.usart);
	CMU_ClockEnable(spi_init.clock, false);
	pinMode(spi_init.ss, INPUT_PULLUP);
	pinMode(spi_init.sck,  INPUT_PULLUP);
	pinMode(spi_init.mosi, INPUT_PULLUP);
	pinMode(spi_init.miso, INPUT_PULLUP);
}

void SPIClass::setBitOrder(uint8_t bitOrder)
{
	if(bitOrder == LSBFIRST)
		spi_init.init.msbf = false;
	else
		spi_init.init.msbf = true;
}

void SPIClass::setDataMode(uint8_t mode)
{
	switch (mode)
	{
		case SPI_MODE0:
			/** Clock idle low, sample on rising edge. */
			spi_init.init.clockMode = usartClockMode0;
			break;
		case SPI_MODE1:
			/** Clock idle low, sample on falling edge. */
			spi_init.init.clockMode = usartClockMode1;
			break;
		case SPI_MODE2:
			/** Clock idle high, sample on falling edge. */
			spi_init.init.clockMode = usartClockMode2;
			break;
		case SPI_MODE3:
			/** Clock idle high, sample on rising edge. */
			spi_init.init.clockMode = usartClockMode3;
			break;
	}
}

void SPIClass::setClockDivider(uint8_t rate)
{
	switch (rate)
	{
		case SPI_CLOCK_DIV2:
			spi_init.init.baudrate = 8000000;
			break;
		case SPI_CLOCK_DIV4:
			spi_init.init.baudrate = 4000000;
			break;
		case SPI_CLOCK_DIV8:
			spi_init.init.baudrate = 2000000;
			break;
		case SPI_CLOCK_DIV16:
			spi_init.init.baudrate = 1000000;
			break;
		case SPI_CLOCK_DIV32:
			spi_init.init.baudrate = 500000;
			break;
		case SPI_CLOCK_DIV64:
			spi_init.init.baudrate = 250000;
			break;
		case SPI_CLOCK_DIV128:
			spi_init.init.baudrate = 125000;
			break;
	}
}

byte SPIClass::transfer(byte _data)
{
	USART_Tx(spi_init.usart, _data);
    return USART_Rx(spi_init.usart);
}

void SPIClass::attachInterrupt()
{
	USART_IntClear(spi_init.usart, _UART_IF_MASK);

	USART_IntEnable(spi_init.usart, UART_IF_RXDATAV);
	USART_IntEnable(spi_init.usart, UART_IF_TXBL);

	NVIC_ClearPendingIRQ(spi_init.RX_IRQn);
	NVIC_ClearPendingIRQ(spi_init.TX_IRQn);

	NVIC_EnableIRQ(spi_init.RX_IRQn);
	NVIC_EnableIRQ(spi_init.TX_IRQn);
}

void SPIClass::detachInterrupt()
{
	USART_IntDisable(spi_init.usart, UART_IF_RXDATAV);
	USART_IntDisable(spi_init.usart, UART_IF_TXBL);
}
