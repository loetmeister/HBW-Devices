/*
 * TimerCounter1.cpp
 *
 *  Created on: 17.07.2017
 *      Author: Viktor Pankraz
 */

#include "TimerCounter1.h"

TimerCounter1& TimerCounter1::instance( uint8_t portNumber )
{
   switch ( portNumber )
   {
    #ifdef TCC1
      case PortC:
         return *reinterpret_cast<TimerCounter1*> ( &TCC1 );
    #endif

    #ifdef TCD1
      case PortD:
         return *reinterpret_cast<TimerCounter1*> ( &TCD1 );
    #endif

    #ifdef TCE1
      case PortE:
         return *reinterpret_cast<TimerCounter1*> ( &TCE1 );
    #endif
   }

   return *(TimerCounter1*)0;
}
