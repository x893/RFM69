#ifndef __USBIO_H__
#define __USBIO_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*USB_RxAccept_TypeDef)(void *cbData, uint8_t * buffer, uint32_t xferred, uint32_t remaining);

int  USB_txByte		(char data);
void USB_txString	(const char *src);
void USB_txBytes	(const uint8_t *src, uint16_t len);
void USB_rxAccept	(USB_RxAccept_TypeDef cbAccept, void *cbData);
bool USB_rxStatusOK	(void);
uint16_t USB_GetControlState(void);

#define PUTCHAR(c)	USB_txByte(c)

#ifdef __cplusplus
}
#endif

#endif	/* __USBIO_H__ */
