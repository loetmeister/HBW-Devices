/*
 * HBWMulltiKeySD6v1Hw.h
 *
 * Created: 09.01.2018 18:07:15
 * Author: viktor.pankraz
 */


#ifndef __HBWMULTIKEYSD6V1HW_H__
#define __HBWMULTIKEYSD6V1HW_H__

#include "HBWMultiKeySD6BaseHw.h"

#include <PortPin.h>

class HBWMultiKeySD6v1Hw : public HBWMultiKeySD6BaseHw
{
// variables
   public:

   protected:

   private:

// functions
   public:
      inline HBWMultiKeySD6v1Hw() : HBWMultiKeySD6BaseHw( PortPin( PortR, 0 ), PortPin( PortR, 1 ), false, true )
      {
         for ( uint8_t i = 0; i < 12; i++ )
         {
            HmwKey* keyChannel = (HmwKey*)HmwChannel::getChannel( i );

            // set ledFeedback channels
            keyChannel->setFeedbackChannel( HmwChannel::getChannel( i + 12 ) );

            // this hardware does not support pull down configuration
            keyChannel->setPulldownSupport( false );
         }
      }

   protected:
   private:


}; // HBWMultiKeySD6v1Hw

#endif // __HBWMULTIKEYSD6V1HW_H__
