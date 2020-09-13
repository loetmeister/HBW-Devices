/*
 * IoPort.cpp
 *
 *  Created on: 17.07.2017
 *      Author: Viktor Pankraz
 */

#include "IoPort.h"

IoPort IoPort::dummyPort;
IoPort IoPort::ports[PortMax];

IoPort& IoPort::instance( uint8_t portNumber )
{
   if ( portNumber < PortMax )
   {
      return ports[portNumber];
   }
   return dummyPort;
}
