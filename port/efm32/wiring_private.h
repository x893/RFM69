#ifndef WiringPrivate_h
#define WiringPrivate_h

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include "Arduino.h"

#ifdef __cplusplus
extern "C"{
#endif

#define EXTERNAL_INT_0 0
#define EXTERNAL_INT_1 1
#define EXTERNAL_INT_2 2
#define EXTERNAL_INT_3 3
#define EXTERNAL_INT_4 4
#define EXTERNAL_INT_5 5
#define EXTERNAL_INT_6 6
#define EXTERNAL_INT_7 7

typedef void (*voidFuncPtr)(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
