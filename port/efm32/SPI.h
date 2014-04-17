/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include <stdio.h>
#include <Arduino.h>

#include "em_device.h"
#include "em_usart.h"
#include "em_cmu.h"

typedef struct SPI_s {
	USART_TypeDef * usart;
	CMU_Clock_TypeDef clock;
	uint32_t route;
	uint8_t mosi;
	uint8_t miso;
	uint8_t sck;
	uint8_t ss;
	enum IRQn RX_IRQn;
	enum IRQn TX_IRQn;
	USART_InitSync_TypeDef init;
} SPI_t;

#define SPI_CLOCK_DIV4		0x00
#define SPI_CLOCK_DIV16		0x01
#define SPI_CLOCK_DIV64		0x02
#define SPI_CLOCK_DIV128	0x03
#define SPI_CLOCK_DIV2		0x04
#define SPI_CLOCK_DIV8		0x05
#define SPI_CLOCK_DIV32		0x06

#define SPI_MODE0	0x00
#define SPI_MODE1	0x04
#define SPI_MODE2	0x08
#define SPI_MODE3	0x0C

class SPIClass {
	private:
		SPI_t spi_init;
		void reinit();
	public:
		SPIClass(uint8_t);
		byte transfer(byte _data);

		// SPI Configuration methods

		void attachInterrupt();
		void detachInterrupt(); // Default

		void begin(); // Default
		void end();

		void setBitOrder(uint8_t);
		void setDataMode(uint8_t);
		void setClockDivider(uint8_t);
};

extern SPIClass SPI;
#endif
