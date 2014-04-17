#include <stdio.h>
#include <stdbool.h>

#include "em_device.h"
#include "em_usb.h"
#include "usbio.h"
#include "config.h"

#define USB_RX_BUF_SIZ 256
#define USB_TX_BUF_SIZ 256

STATIC_UBUF( usbRxBuffer, USB_RX_BUF_SIZ );
STATIC_UBUF( usbTxBuffer, USB_TX_BUF_SIZ );

#if defined(__CC_ARM)
int fputc(int ch, FILE *f)
{
	return USB_txByte(ch);
}
#else
	#error Unknown compiler
#endif

USB_Status_TypeDef		usbRxStatus = USB_STATUS_TIMEOUT;
USB_RxAccept_TypeDef	usbRxAcceptCallback;
void * UsbRxData;

/**********************************************************
 * Called when data is received on the OUT endpoint. 
 * This function will increase the counter and update
 * the LCD display when it receives a 'tick' message
 * to let the user know that the message was received
 * (only on STK example).
 * 
 * @param status
 *   The transfer status. Should be USB_STATUS_OK if the
 *   transfer completed successfully.
 * 
 * @param xferred
 *   The number of bytes actually received
 * 
 * @param remaining
 *   The number of bytes remaining (not transferred)
 **********************************************************/
int UsbRxComplete(USB_Status_TypeDef status,
						uint32_t xferred,
						uint32_t remaining
						)
{
	usbRxStatus = status;
	if ( status == USB_STATUS_OK )
	{
		if (usbRxAcceptCallback != NULL)
		{
			usbRxAcceptCallback(UsbRxData, usbRxBuffer, xferred, remaining);
			/* Prepare to accept the next message */
			USBD_Read(EP_DATA_OUT, usbRxBuffer, USB_RX_BUF_SIZ, UsbRxComplete);
		}
	}
	return USB_STATUS_OK;
}

bool USB_rxStatusOK(void)
{
	return (usbRxStatus == USB_STATUS_OK);
}

void USB_rxAccept(USB_RxAccept_TypeDef cbAccept, void *cbData)
{
	int retry = 10;
	usbRxAcceptCallback = cbAccept;
	UsbRxData = cbData;
	if (cbAccept != NULL)
	{
		while ( retry-- != 0 &&
				USB_STATUS_OK != USBD_Read( EP_DATA_OUT, usbRxBuffer, USB_RX_BUF_SIZ, UsbRxComplete )
				)
		{
			USBTIMER_DelayMs( 100 );
		}
		if (retry != 0)
			usbRxStatus = USB_STATUS_OK;
	}
}

/**************************************************************************//**
 * @brief
 *    Callback function called whenever a packet with data has been
 *    transferred on USB.
 *****************************************************************************/
volatile bool      usbTxDone;
USB_Status_TypeDef usbTxStatus;

static int UsbTxComplete(USB_Status_TypeDef status,
						uint32_t xferred,
						uint32_t remaining
						)
{
	(void)remaining;            /* Unused parameter */
	usbTxStatus = status;
	usbTxDone   = true;
	return USB_STATUS_OK;
}

/**************************************************************************//**
 * @brief Transmit single byte to USART or USB
 *****************************************************************************/
int USB_txByte( char data )
{
	usbTxBuffer[ 0 ] = data;
	usbTxDone = false;
	USBD_Write( EP_DATA_IN, usbTxBuffer, 1, UsbTxComplete );
	while ( !usbTxDone ) { }
	return (usbTxStatus == USB_STATUS_OK ? (int)data : 0);
}

/**************************************************************************//**
 * @brief Transmit null-terminated string to USART or USB
 *****************************************************************************/
void USB_txString( const char *src )
{
	uint16_t len = strlen(src);
	USB_txBytes((const uint8_t *)src, len);
}

void USB_txBytes  ( const uint8_t *src, uint16_t len )
{
	while (len != 0)
	{
		int size = sizeof(usbTxBuffer);
		size = (len > size ? size : len);
		len -= size;

		memcpy( usbTxBuffer, src, size );
		usbTxDone = false;
		USBD_Write( EP_DATA_IN, usbTxBuffer, size, UsbTxComplete );
		while ( !usbTxDone ) { }
	}
}
