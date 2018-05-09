/*
 * HmwBrightness.cpp
 *
 *  Created on: 07.05.2018
 *      Author: 
 */

#include "HmwBrightness.h"
#include "HmwDevice.h"
#include "HmwAnalogIn.h"

HmwBrightness::HmwBrightness( HmwAnalogIn& _linkedAnalogChannel, Config* _config ) :
   config( _config ),
   linkedAnalogChannel( &_linkedAnalogChannel )
{
   nextActionDelay = 6130;	// start after analog sampling started
   lastActionTime = 0;
   currentValue = 0;
}


uint8_t HmwBrightness::get( uint8_t* data )
{
   *data = currentValue;
   return 1;
}

void HmwBrightness::loop( uint8_t channel )
{
   if ( !nextActionDelay )
   {
      return;
   }

   if ( lastActionTime.since() < nextActionDelay )
   {
      return;
   }

   lastActionTime = Timestamp();   // at least last time of trying
   
   static uint8_t buffer[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
   static uint16_t result_sum = 0;
   static uint8_t index = 0;

   uint8_t reading, data[2];
   linkedAnalogChannel->get( data );
//   reading = (uint8_t) ( (uint16_t)( (data[1] <<8) & data[0]) / 20 );
   reading = (((uint16_t) data[0] <<8) | data[1]) / 20 ;
	
   /* calculate the average of the last n results */
   result_sum += (reading - buffer[index]);  // add new and subtract old value
   buffer[index] = reading;  // update buffer with current reading
   currentValue = result_sum / config->samples;  // calculate average

   ++index;
   if (index > config->samples) {
	   index = 0; // reset when last array element was processed
   }

   lastSentTime = Timestamp();

   // start next measurement after x seconds (should be 8 [analogIn sample interval] to 255 seconds)
   nextActionDelay = config->interval* 1000;
}
