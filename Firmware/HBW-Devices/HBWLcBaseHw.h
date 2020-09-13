/*
 * HBWLcBaseHw.h
 *
 * Created: 09.01.2018 18:36:42
 * Author: viktor.pankraz
 */


#ifndef __HBWLCBASEHW_H__
#define __HBWLCBASEHW_H__

#include "HBWGenericDeviceHw.h"

#include <PortPin.h>


class HBWLcBaseHw : public HBWGenericDeviceHw
{
// variables
   public:

   protected:

   private:

      DigitalOutputTmpl<PortA, 5> txEnable;
      DigitalOutputTmpl<PortA, 6> rxEnable;
      DigitalOutputTmpl<PortR, 0> greenLed;
      DigitalOutputTmpl<PortR, 1> configLed;
      DigitalInputTmpl<PortE, 4> configButton;

// functions
   public:

      inline HBWLcBaseHw()
      {
         configLed.setInverted();
         configButton.enablePullup();
      }

      virtual inline void enableTranceiver( bool enable )
      {
         enable ? txEnable.set() : txEnable.clear();
         enable ? greenLed.set() : greenLed.clear();
      }

      virtual inline bool isConfigButtonPressed()
      {
         return !configButton.isSet();
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
                  lastLEDtime = Timestamp();
               }
               break;
            }

            case SECOND_LONG_PRESS:
            {
               if ( lastLEDtime.since() > 200 )
               {
                  configLed.toggle();
                  lastLEDtime = Timestamp();
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

}; // HBWLcBaseHw

#endif // __HBWLCBASEHW_H__
