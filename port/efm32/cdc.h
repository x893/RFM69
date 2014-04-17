#ifndef __CDC_H__
#define __CDC_H__

#include "em_usb.h"

#ifdef __cplusplus
extern "C" {
#endif

extern bool CDC_Configured;

int  CDC_SetupCmd( const USB_Setup_TypeDef *setup );
void CDC_StateChange( USBD_State_TypeDef oldState, USBD_State_TypeDef newState );

#ifdef __cplusplus
}
#endif

#endif
