/*
 * TimerCounter.cpp
 *
 *  Created on: 17.07.2017
 *      Author: Viktor Pankraz
 */

#include "TimerCounter.h"
#include <SwFramework.h>

TimerCounter& TimerCounter::instance( uint8_t portNumber, uint8_t type )
{
   WARN_3( FSTR( "Requested Timer not exists! port" ), (uint8_t)( 'A' + portNumber ), type );
   return *(TimerCounter*)0;
}

