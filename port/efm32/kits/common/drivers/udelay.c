/**************************************************************************//**
 * @file udelay.c
 * @brief Microsecond delay routine.
 * @author Energy Micro AS
 * @version 3.20.2
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2013 Energy Micro AS, http://www.energymicro.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 4. The source and compiled code may only be used on Energy Micro "EFM32"
 *    microcontrollers and "EFR4" radios.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 *****************************************************************************/
#include "em_device.h"
#include "em_cmu.h"
#include "em_int.h"
#include "em_rtc.h"

#include "udelay.h"

/**************************************************************************//**
 * @addtogroup Udelay
 * @{ Implements active wait microsecond delay.
 *
 *  The delay is implemented as a loop coded in assembly. The delay loop must
 *  be calibrated by calling @ref UDELAY_Calibrate() once. The calibration
 *  algorithm is taken from linux 2.4 sources (bogomips).
 *
 *  The delay is fairly accurate, the assembly coding will not be optimized
 *  by the compiler.
 *  Recalibrate the loop when HFCORECLK is changed.
 *
 *  The calibration uses the RTC clocked by LFRCO to measure time. Better
 *  accuracy can be achieved by adding \#define UDELAY_LFXO (i.e. add
 *  -DUDELAY_LFXO on the commandline). The LFXO oscillator is then used for
 *  delay loop calibration.
 *
 *  The calibration function will restore RTC upon exit.
 ** @} ***********************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/* this should be approx 2 Bo*oMips to start (note initial shift), and will
 *    still work even if initially too large, it will just take slightly longer */
volatile unsigned long loops_per_jiffy = (1<<12);

/* This is the number of bits of precision for the loops_per_jiffy.  Each
 *    bit takes on average 1.5/HZ seconds.  This (like the original) is a little
 *       better than 1% */
#define LPS_PREC 8

static void calibrate_delay(void);
__STATIC_INLINE uint32_t clock(void);
static void _delay( uint32_t delay);

/** @endcond */

/***************************************************************************//**
 * @brief
 *   Calibrates the microsecond delay loop.
 ******************************************************************************/
void UDELAY_Calibrate(void)
{
	CMU_Select_TypeDef lfaClkSel;
	CMU_ClkDiv_TypeDef rtcClkDiv;
	bool rtcRestore       = false;
	bool leClkTurnoff     = false;
	bool rtcClkTurnoff    = false;
	bool lfaClkSrcRestore = false;
	bool lfaClkTurnoff    = false;
	RTC_Init_TypeDef init = RTC_INIT_DEFAULT;
	uint32_t rtcCtrl=0, rtcComp0=0, rtcComp1=0, rtcIen=0;

	/* Ensure LE modules are accessible */
	if ( !( CMU->HFCORECLKEN0 & CMU_HFCORECLKEN0_LE) )
	{
		CMU_ClockEnable(cmuClock_CORELE, true);
		leClkTurnoff = true;
	}

	lfaClkSel = CMU_ClockSelectGet(cmuClock_LFA);

#if defined( UDELAY_LFXO )
	if ( !(CMU->STATUS & CMU_STATUS_LFXOENS) )
	{
		lfaClkTurnoff = true;
		CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
	}

	if ( lfaClkSel != cmuSelect_LFXO )
	{
		lfaClkSrcRestore = true;
		CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
	}

#else
	if ( lfaClkSel != cmuSelect_LFRCO )
	{
		lfaClkSrcRestore = true;
	}
	if ( !(CMU->STATUS & CMU_STATUS_LFRCOENS) )
	{
		lfaClkTurnoff = true;
	}
	/* Enable LFACLK in CMU (will also enable oscillator if not enabled) */
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFRCO);
#endif

	/* Set up a reasonable prescaler. */
	rtcClkDiv = CMU_ClockDivGet(cmuClock_RTC);
	CMU_ClockDivSet(cmuClock_RTC, cmuClkDiv_256);

	if ( !(CMU->LFACLKEN0 & CMU_LFACLKEN0_RTC) )
	{
		/* Enable clock to RTC module */
		CMU_ClockEnable(cmuClock_RTC, true);
		rtcClkTurnoff = true;
	}

	INT_Disable();

	if ( RTC->CTRL & RTC_CTRL_EN )
	{
		/* Stash away current RTC settings. */
		rtcCtrl   = RTC->CTRL;
		rtcComp0  = RTC->COMP0;
		rtcComp1  = RTC->COMP1;
		rtcIen    = RTC->IEN;

		RTC->CTRL = _RTC_CTRL_RESETVALUE;
		RTC->IEN  = 0;
		RTC->IFC  = _RTC_IEN_MASK;

		NVIC_ClearPendingIRQ( RTC_IRQn );

		rtcRestore = true;
	}

	init.comp0Top = false;  /* Count to max before wrapping */
	RTC_Init(&init);        /* Start RTC counter. */

	calibrate_delay();      /* Calibrate the micro second delay loop. */

	INT_Enable();

	/* Restore all RTC related settings to how they were previously set. */
	if ( rtcRestore )
	{
		CMU_ClockDivSet(cmuClock_RTC, rtcClkDiv);

		RTC_FreezeEnable(true);
#if defined(_EFM32_GECKO_FAMILY)
		RTC_Sync(RTC_SYNCBUSY_COMP0 | RTC_SYNCBUSY_COMP1 | RTC_SYNCBUSY_CTRL);
#endif
		RTC->COMP0 = rtcComp0;
		RTC->COMP1 = rtcComp1;
		RTC->CTRL  = rtcCtrl;
		RTC->IEN   = rtcIen;
		RTC_FreezeEnable(false);
	}
	else
	{
		RTC_Enable(false);
	}

	if ( rtcClkTurnoff )
	{
		CMU_ClockEnable(cmuClock_RTC, false);
	}

	if ( lfaClkSrcRestore )
	{
		CMU_ClockSelectSet(cmuClock_LFA, lfaClkSel);
	}

	if ( lfaClkTurnoff )
	{
#if defined( UDELAY_LFXO )
		CMU_OscillatorEnable(cmuOsc_LFXO, false, false);
#else
		CMU_OscillatorEnable(cmuOsc_LFRCO, false, false);
#endif
	}

	if ( leClkTurnoff )
	{
		CMU_ClockEnable(cmuClock_CORELE, false);
	}
}

#if defined(__GNUC__) /* GCC */
/***************************************************************************//**
 * @brief
 *   Microsecond active wait delay routine.
 *
 * @param[in] usecs
 *   Number of microseconds to delay.
 ******************************************************************************/
void UDELAY_Delay( uint32_t usecs )
{
  __ASM volatile (
#if defined(_EFM32_ZERO_FAMILY)
"        .syntax unified           \n"
"        .arch armv6-m             \n"
#endif
"        movs    r2, #0x88         \n"
"        lsls    r2, r2, #8        \n"
"        adds    r2, #0x00         \n"
"        muls    %0, r2            \n"
"                                  \n"
"        ldr     r2, [%1]          \n"
"        movs    r0, %0, lsr #11   \n"
"        movs    r2, r2, lsr #11   \n"
"                                  \n"
"        muls    r0, r2            \n"
"        movs    r0, r0, lsr #6    \n"
"                                  \n"
"        beq.n   2f                \n"
"                                  \n"
"1:      subs    r0, #1            \n"
"        bhi     1b                \n"
#if defined(_EFM32_ZERO_FAMILY)
"2:                                \n"
"        .syntax divided           \n" : : "r" (usecs), "r" (&loops_per_jiffy) );
#else
"2:                                \n" : : "r" (usecs), "r" (&loops_per_jiffy) );
#endif
}
#endif /* defined(__GNUC__) */

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

static void calibrate_delay(void)
{
  /* From linux 2.4 source. */
  unsigned long loopbit;
  unsigned long ticks;
  int lps_precision = LPS_PREC;

  loops_per_jiffy = (1<<12);

  while (loops_per_jiffy <<= 1) {
    /* wait for "start of" clock tick */
    ticks = clock();
    while (ticks == clock())
      /* nothing */;
    /* Go .. */
    ticks = clock();
    _delay(loops_per_jiffy);
    ticks = clock() - ticks;
    if (ticks)
      break;
  }

  /* Do a binary approximation to get loops_per_jiffy set to equal one clock
     (up to lps_precision bits) */

  loops_per_jiffy >>= 1;
  loopbit = loops_per_jiffy;
  while ( lps_precision-- && (loopbit >>= 1) ) {
    loops_per_jiffy |= loopbit;
    ticks = clock();
    while (ticks == clock());
    ticks = clock();
    _delay(loops_per_jiffy);
    if (clock() != ticks)   /* longer than 1 tick */
      loops_per_jiffy &= ~loopbit;
  }
}

__STATIC_INLINE uint32_t clock(void)
{
  return RTC_CounterGet();
}

#if defined(__ICCARM__) /* IAR */
static void _delay( uint32_t delay)
{
  __ASM volatile (
"_delay_1:                         \n"
"        subs    r0, #1            \n"
"        bhi.n   _delay_1          \n" );
}

void UDELAY_Delay( uint32_t usecs )
{
  __ASM volatile (
"        movs    r2, #0x88         \n"
"        lsls    r2, r2, #8        \n"
"        adds    r2, #0x00         \n"
"        muls    r0, r2            \n"
"                                  \n"
"        ldr     r2, [%0]          \n"
"        movs    r0, r0, lsr #11   \n"
"        movs    r2, r2, lsr #11   \n"
"                                  \n"
"        muls    r0, r2            \n"
"        movs    r0, r0, lsr #6    \n"
"                                  \n"
"        bne.n   udelay_1          \n"
"        bx      lr                \n"
"                                  \n"
"udelay_1:                         \n"
"        subs    r0, #1            \n"
"        bhi.n   udelay_1          \n" : : "r" (&loops_per_jiffy) );
}
#endif /* defined(__ICCARM__) */

#if defined(__GNUC__) /* GCC */
static void _delay( uint32_t delay )
{
  __ASM volatile (
#if defined(_EFM32_ZERO_FAMILY)
"        .syntax unified           \n"
"        .arch armv6-m             \n"
#endif
"1:      subs    %0, #1            \n"
#if defined(_EFM32_ZERO_FAMILY)
"        bhi.n   1b                \n"
"        .syntax divided           \n" : : "r" (delay) );
#else
"        bhi.n   1b                \n" : : "r" (delay) );
#endif
}
#endif /* defined(__GNUC__) */

#if defined(__CC_ARM) /* Keil */
static __ASM void _delay( uint32_t delay)
{
_delay_1
        subs    r0, #1
        bhi     _delay_1
        bx      lr
}

__ASM void UDELAY_Delay( uint32_t usecs __attribute__ ((unused)) )
{
        IMPORT  loops_per_jiffy

        movs    r2, #0x88
        lsls    r2, r2, #8
        adds    r2, #0x00
        muls    r0, r2, r0

        ldr     r2, =loops_per_jiffy
        ldr     r2, [r2]
        movs    r0, r0, lsr #11
        movs    r2, r2, lsr #11

        muls    r0, r2, r0
        movs    r0, r0, lsr #6

        bne     udelay_1
        bx      lr

udelay_1
        subs    r0, #1
        bhi     udelay_1
        bx      lr
}
#endif /* defined(__CC_ARM) */

/** @endcond */
