/*
 * TimerCounterBase.h
 *
 *  Created on: 17.07.2017
 *      Author: Viktor Pankraz
 */

#ifndef Peripherals_TimerCounter_H
#define Peripherals_TimerCounter_H

#include <DefaultTypes.h>

class TimerCounter
{
   public:

      enum Channel
      {
         A = 0,
         B = 1,
         C = 2,
         D = 3
      };

      ////    Constructors and destructors    ////

   protected:

      inline TimerCounter()
      {
      }

      ////    Operations    ////

   public:

      static TimerCounter& instance( uint8_t portNumber, uint8_t type );

      static inline Channel getChannelFromPinNumber( uint8_t pinNumber )
      {
         if ( pinNumber >= 4 )
         {
            pinNumber -= 4;
         }
         return (Channel)pinNumber;
      }

      inline void clearCCAFlag()
      {
      }

      inline void clearCCBFlag()
      {
      }

      inline void clearErrorFlag()
      {
      }

      inline void clearOverflowFlag()
      {
      }

      inline void disableEventDelay()
      {
      }

      inline void enableEventDelay()
      {
      }

      inline void forceUpdate()
      {
      }

      inline uint16_t getCount()
      {
         return 0;
      }

      inline uint8_t getCCAFlag()
      {
         return 0;
      }

      inline uint8_t getCCBFlag()
      {
         return 0;
      }

      inline uint16_t getCaptureA()
      {
         return 0;
      }

      inline uint16_t getCaptureB()
      {
         return 0;
      }

      inline uint8_t getErrorFlag()
      {
         return 0;
      }

      inline uint8_t getOverflowFlag()
      {
         return 0;
      }

      inline uint16_t getPeriod()
      {
         return 0;
      }

      inline uint8_t isRunning()
      {
         return true;
      }

      inline void lockCompareUpdate()
      {
      }

      inline void reset()
      {
      }

      inline void restart()
      {
      }

      inline void setCount( uint16_t value )
      {
      }

      inline void setPeriod( uint16_t value )
      {
      }

      inline void unlockCompareUpdate()
      {
      }

      inline void clearCCFlag( Channel channel )
      {
      }

      inline uint8_t getCCFlag( Channel channel )
      {
         return 0;
      }

      inline uint16_t getCapture( Channel channel )
      {
         return 0;
      }

      inline uint16_t getCaptureBuffered( Channel channel )
      {
         return 0;
      }

      inline void disableChannel( Channel channel )
      {
      }

      inline void enableChannel( Channel channel )
      {
      }

      inline void setCompare( Channel channel, uint16_t value )
      {
      }

      inline void setCompareBuffered( Channel channel, uint16_t value )
      {
      }

      inline void clearCCCFlag()
      {
      }

      inline void clearCCDFlag()
      {
      }

      inline void disableChannels( uint8_t disableMask )
      {
      }

      inline void enableChannels( uint8_t enableMask )
      {
      }

      inline uint8_t getCCCFlag()
      {
         return 0;
      }

      inline uint8_t getCCDFlag()
      {
         return 0;
      }

      inline uint16_t getCaptureC()
      {
         return 0;
      }

      inline uint16_t getCaptureD()
      {
         return 0;
      }


      inline void setCompareA( uint16_t value )
      {
      }

      inline void setCompareB( uint16_t value )
      {
      }

      inline void setCompareC( uint16_t value )
      {
      }

      inline void setCompareD( uint16_t value )
      {
      }

      inline void setPeriodBuffered( uint16_t value )
      {
      }
      ////    Attributes    ////

   protected:

};

#endif
