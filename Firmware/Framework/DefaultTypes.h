/*
 * DefaultTypes.h
 *
 *  Created on: 24.06.2014
 *      Author: viktor.pankraz
 */

#ifndef DefaultTypes_DefaultTypes_H
#define DefaultTypes_DefaultTypes_H

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum PinMask
{
   NoPinMask = 0x00,
   Pin0Mask = 0x01,
   Pin1Mask = 0x02,
   Pin2Mask = 0x04,
   Pin3Mask = 0x08,
   Pin4Mask = 0x10,
   Pin5Mask = 0x20,
   Pin6Mask = 0x40,
   Pin7Mask = 0x80,
   AllPinsMask = 0xFF
};

enum PinId
{
   Pin0,
   Pin1,
   Pin2,
   Pin3,
   Pin4,
   Pin5,
   Pin6,
   Pin7,
   PinMax
};

enum Port
{
   Port0 = 0,
   Port1,
   Port2,
   Port3,
   Port4,
   Port5,
   Port6,
   Port7,
   Port8,
   Port9,
   Port10,
   Port11,
   Port12,
   Port13,

   PortA = 0,
   PortB,
   PortC,
   PortD,
   PortE,
   PortF,
   PortG,
   PortH,
   PortI,
   PortJ,
   PortK,
   PortQ,
   PortR,
   PortDummy,
   PortMax,
};

typedef void (* pFunction)();


#if ( ( __GNUC__ * 100 ) + __GNUC_MINOR__ ) >= 402
#define GCC_DIAG_STR( s ) #s
#define GCC_DIAG_JOINSTR( x, y ) GCC_DIAG_STR( x ## y )
# define GCC_DIAG_DO_PRAGMA( x ) _Pragma(#x )
# define GCC_DIAG_PRAGMA( x ) GCC_DIAG_DO_PRAGMA( GCC diagnostic x )
# if ( ( __GNUC__ * 100 ) + __GNUC_MINOR__ ) >= 406
# define GCC_DIAG_OFF( x ) GCC_DIAG_PRAGMA( push ) \
   GCC_DIAG_PRAGMA( ignored GCC_DIAG_JOINSTR( -W, x ) )
# define GCC_DIAG_ON( x ) GCC_DIAG_PRAGMA( pop )
# else
# define GCC_DIAG_OFF( x ) GCC_DIAG_PRAGMA( ignored GCC_DIAG_JOINSTR( -W, x ) )
# define GCC_DIAG_ON( x ) GCC_DIAG_PRAGMA( warning GCC_DIAG_JOINSTR( -W, x ) )
# endif
#else
# define GCC_DIAG_OFF( x )
# define GCC_DIAG_ON( x )
#endif

#endif
