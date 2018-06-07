/*
 * HmwBrightness.cpp
 *
 *  Created on: 07.05.2018
 *      Author: loetmeister.de
 */

#include "HmwBrightness.h"
#include "HmwDevice.h"
#include "HmwAnalogIn.h"


HmwBrightness::HmwBrightness( HmwAnalogIn& _linkedAnalogChannel, Config* _config ) :
   config( _config ),
   linkedAnalogChannel( &_linkedAnalogChannel )
{
   nextActionDelay = 8694;	// start after analog sampling completed once
   lastActionTime = 0;
   currentValue = 0;
   result_sum = 0;
   index = 0;
   count = 0;
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

   lastActionTime = Timestamp();

   uint8_t reading;
   reading = linkedAnalogChannel->currentValue / 20;
   //if (reading > 200) reading = 200;
	
   /* calculate the average of the last n results */
   result_sum -= buffer[index];  // subtract old value
   buffer[index] = reading;  // update buffer with current reading
   result_sum += buffer[index];  // add new value
   index++;
   index = index % MAX_SAMPLES; // reset when last array element was processed
   if (count < MAX_SAMPLES) count++;
   
   if (result_sum > 0)
	currentValue = result_sum / count;  // calculate average
   else
	currentValue = 0;


   // start next measurement after x seconds (should be 8 [analogIn sample interval] to 255 seconds)
   nextActionDelay = config->interval* 1000;
}
