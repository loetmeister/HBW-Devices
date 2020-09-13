/*
 * DigitalOutput.h
 *
 * Created: 18.06.2014 14:12:56
 * Author: viktor.pankraz
 */

#ifndef Basics_DigitalOutput_H
#define Basics_DigitalOutput_H

#include "DigitalInput.h"

class DigitalOutput : public DigitalInput
{
   ////    Constructors and destructors    ////

   public:

      inline DigitalOutput( uint8_t _portNumber, uint8_t _pinNumber, bool config = true ) : DigitalInput( _portNumber, _pinNumber, false )
      {
         if ( isValid() && config )
         {
            configOutput();
         }
      }

      inline DigitalOutput( PortPin _portPin, bool config = true ) : DigitalInput( _portPin, false )
      {
         if ( isValid() && config )
         {
            configOutput();
         }
      }

      ////    Operations    ////

      void clear();

      void set();

      void toggle();
};

template<uint8_t portNumber, uint8_t pinNumber>
class DigitalOutputTmpl : public DigitalInputTmpl<portNumber, pinNumber>
{
   ////    Constructors and destructors    ////

   public:

      inline DigitalOutputTmpl()
      {
         if ( this->isValid() )
         {
            this->configOutput();
         }
      }

      ////    Operations    ////

      inline void clear()
      {
         this->getIoPort().clearPins( this->getPin() );
      }

      inline void set()
      {
         this->getIoPort().setPins( this->getPin() );
      }

      inline void toggle()
      {
         this->getIoPort().togglePins( this->getPin() );
      }
};
#endif

