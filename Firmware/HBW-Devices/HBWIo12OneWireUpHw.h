/*
 * HBWIo12OneWireUpHw.h
 *
 * Created: 09.01.2018 18:07:15
 * Author: viktor.pankraz
 */


#ifndef __HBW_IO12_1W_UP_HW_H__
#define __HBW_IO12_1W_UP_HW_H__

#include "HBWMultiKeySD6BaseHw.h"

#include <DigitalOutput.h>

class HBWIo12OneWireUpHw : public HBWMultiKeySD6BaseHw
{
// variables
   public:

   protected:

   private:

// functions
   public:
      inline HBWIo12OneWireUpHw() : HBWMultiKeySD6BaseHw( PortPin( PortR, 0 ), PortPin( PortR, 1 ), false, false )
      {
         // disable not available channels on this hardware
         //hbwOnboardBrightness.disable();
		 hbwAnIn1.disable();
		 hbwAnIn2.disable();
         //shtc3Temp.disable();
		 sht3x.disable();

         // config specific IOs
         configLed.setInverted( true );
         configbutton.enablePullup();

         for ( uint8_t i = 0; i < 12; i++ )
         {
            HmwKey* keyChannel = (HmwKey*)HmwChannel::getChannel( i );

//TODO: useful to add feedback channels? Possible to disable by default?
            // set ledFeedback channels
            //keyChannel->setFeedbackChannel( HmwChannel::getChannel( i + 12 ) );

            // this hardware does not support pull down configuration
            keyChannel->setPulldownSupport( false );
         }
      }

      virtual inline bool isConfigButtonPressed()
      {
         return !configbutton.isSet();
      }

      virtual inline void notifyConfigButtonState( ConfigButtonState state )
      {
         static Timestamp lastLEDtime;

         switch ( state )
         {
            case IDLE:
            {
               configLed.clear();
               break;
            }

            case FIRST_PRESS:
            {
               configLed.set();
               break;
            }

            case FIRST_LONG_PRESS:
            case WAIT_SECOND_PRESS:
            case SECOND_PRESS:
            {
               if ( lastLEDtime.since() > 500 )
               {
                  configLed.toggle();
                  lastLEDtime.setNow();
               }
               break;
            }

            case SECOND_LONG_PRESS:
            {
               if ( lastLEDtime.since() > 200 )
               {
                  configLed.toggle();
                  lastLEDtime.setNow();
               }
               break;
            }
            default:
            {
               // nothing to do
            }
         }
      }
   protected:
   private:
      DigitalOutputTmpl<PortA, 6> configLed;

      DigitalInputTmpl<PortA, 7> configbutton;


}; // HBWIo12OneWireUpHw

#endif // __HBW_IO12_1W_UP_HW_H__
