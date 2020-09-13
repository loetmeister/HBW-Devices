/*
 * PwmOutput.h
 *
 *  Created on: 17.07.2017
 *      Author: Viktor Pankraz
 */

#ifndef Basics_PwmOutput_H
#define Basics_PwmOutput_H

#include "DigitalOutput.h"


class PwmOutput : public DigitalOutput
{
   ////    Constructors and destructors    ////

   public:
      inline PwmOutput( PortPin _portPin, bool config = true ) : DigitalOutput( _portPin, config )
      {
      }

      inline PwmOutput( uint8_t _portNumber, uint8_t _pinNumber, bool config = true ) : DigitalOutput( _portNumber, _pinNumber, config )
      {
      }

      PwmOutput( uint8_t _portNumber, uint8_t _pinNumber, uint16_t _period );

      ////    Operations    ////

      inline void clear()
      {
         set( 0 );
      }

      uint16_t isSet() const
      {
         return value;
      }

      bool isRunning() const
      {
         return true;
      }

      void set( uint16_t _value )
      {
         value = _value;
         value ? DigitalOutput::set() : DigitalOutput::clear();
      }

      void setPeriode( uint16_t _period )
      {
         period = _period;
      }

      uint16_t getPeriode() const
      {
         return period;
      }

   protected:

      uint16_t value;

      uint16_t period;

};

#endif

