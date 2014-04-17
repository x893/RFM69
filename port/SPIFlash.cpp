#include <SPIFlash.h>

SPIFlash::SPIFlash(uint8_t slaveSelectPin, uint16_t jedecID)
{
	_slaveSelectPin = slaveSelectPin;
	_jedecID = jedecID;
}

/// Select the flash chip
void SPIFlash::select()
{
	noInterrupts();
	digitalWrite(_slaveSelectPin, LOW);
}

/// UNselect the flash chip
void SPIFlash::unselect()
{
	digitalWrite(_slaveSelectPin, HIGH);
	interrupts();
}

/// setup SPI, read device ID etc...
boolean SPIFlash::initialize()
{
	pinMode(_slaveSelectPin, OUTPUT);
	unselect();
	SPI.setDataMode(SPI_MODE0);
	SPI.setBitOrder(MSBFIRST);
	SPI.setClockDivider(SPI_CLOCK_DIV2); //max speed, except on Due which can run at system clock speed
	SPI.begin();

	if (_jedecID == 0 || readDeviceId() == _jedecID)
	{
		command(SPIFLASH_STATUSWRITE, true); // Write Status Register
		SPI.transfer(0);                     // Global Unprotect
		unselect();
		return true;
	}
	return false;
}

/// Get the manufacturer and device ID bytes (as a short word)
word SPIFlash::readDeviceId()
{
	select();
	SPI.transfer(SPIFLASH_IDREAD);
	word jedecid = SPI.transfer(0) << 8;
	jedecid |= SPI.transfer(0);
	unselect();
	return jedecid;
}

/// read 1 byte from flash memory
byte SPIFlash::readByte(long addr)
{
	command(SPIFLASH_ARRAYREADLOWFREQ);
	SPI.transfer(addr >> 16);
	SPI.transfer(addr >> 8);
	SPI.transfer(addr);
	byte result = SPI.transfer(0);
	unselect();
	return result;
}

/// read unlimited # of bytes
void SPIFlash::readBytes(long addr, void* buf, word len)
{
	command(SPIFLASH_ARRAYREAD);
	SPI.transfer(addr >> 16);
	SPI.transfer(addr >> 8);
	SPI.transfer(addr);
	SPI.transfer(0); //"dont care"
	for (word i = 0; i < len; ++i)
		((byte*) buf)[i] = SPI.transfer(0);
	unselect();
}

/// Send a command to the flash chip, pass TRUE for isWrite when its a write command
void SPIFlash::command(byte cmd, boolean isWrite)
{
	if (isWrite)
	{
		command(SPIFLASH_WRITEENABLE); // Write Enable
		unselect();
	}
	while(busy()); //wait for any write/erase to complete
	select();
	SPI.transfer(cmd);
}

/// check if the chip is busy erasing/writing
boolean SPIFlash::busy()
{
	return readStatus() & 1;
}

/// return the STATUS register
byte SPIFlash::readStatus()
{
	select();
	SPI.transfer(SPIFLASH_STATUSREAD);
	byte status = SPI.transfer(0);
	unselect();
	return status;
}

/// Write 1 byte to flash memory
/// WARNING: you can only write to previously erased memory locations (see datasheet)
///          use the block erase commands to first clear memory (write 0xFFs)
void SPIFlash::writeByte(long addr, uint8_t byt)
{
	command(SPIFLASH_BYTEPAGEPROGRAM, true);  // Byte/Page Program
	SPI.transfer(addr >> 16);
	SPI.transfer(addr >> 8);
	SPI.transfer(addr);
	SPI.transfer(byt);
	unselect();
}

/// write 1-256 bytes to flash memory
/// WARNING: you can only write to previously erased memory locations (see datasheet)
///          use the block erase commands to first clear memory (write 0xFFs)
/// WARNING: if you write beyond a page boundary (or more than 256bytes),
///          the bytes will wrap around and start overwriting at the beginning of that same page
///          see datasheet for more details
void SPIFlash::writeBytes(long addr, const void* buf, uint8_t len)
{
	command(SPIFLASH_BYTEPAGEPROGRAM, true);  // Byte/Page Program
	SPI.transfer(addr >> 16);
	SPI.transfer(addr >> 8);
	SPI.transfer(addr);
	for (uint8_t i = 0; i < len; i++)
		SPI.transfer(((byte*) buf)[i]);
	unselect();
}

/// erase entire flash memory array
/// may take several seconds depending on size, but is non blocking
/// so you may wait for this to complete using busy() or continue doing
/// other things and later check if the chip is done with busy()
/// note that any command will first wait for chip to become available using busy()
/// so no need to do that twice
void SPIFlash::chipErase()
{
	command(SPIFLASH_CHIPERASE, true);
	unselect();
}

/// erase a 4Kbyte block
void SPIFlash::blockErase4K(long addr)
{
	command(SPIFLASH_BLOCKERASE_4K, true); // Block Erase
	SPI.transfer(addr >> 16);
	SPI.transfer(addr >> 8);
	SPI.transfer(addr);
	unselect();
}

/// erase a 32Kbyte block
void SPIFlash::blockErase32K(long addr)
{
	command(SPIFLASH_BLOCKERASE_32K, true); // Block Erase
	SPI.transfer(addr >> 16);
	SPI.transfer(addr >> 8);
	SPI.transfer(addr);
	unselect();
}

void SPIFlash::sleep()
{
	command(SPIFLASH_SLEEP); // Block Erase
	unselect();
}

void SPIFlash::wakeup()
{
	command(SPIFLASH_WAKE); // Block Erase
	unselect();
}

/// cleanup
void SPIFlash::end()
{
	SPI.end();
}
