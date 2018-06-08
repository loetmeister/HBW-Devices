/*
 * HmwBrightness.cpp
 *
 *  Created on: 07.06.2018
 *      Author: loetmeister.de
 */

#include "HmwBrightnessSwitch.h"
#include "HmwDevice.h"
#include "HmwAnalogIn.h"


HmwBrightnessSwitch::HmwBrightnessSwitch( HmwAnalogIn& _linkedAnalogChannel, Config* _config ) :
   config( _config ),
   linkedAnalogChannel( &_linkedAnalogChannel )
{
   //nextActionDelay = 1800;	// don't start before analog sampling (->HmwAnalogIn)
   lastActionTime = 0;
   currentValue = 0;
   result_sum = 0;
   index = 0;
   count = 0;
   triggered = false;
}


// allow to read the level (not the triggered state)
uint8_t HmwBrightnessSwitch::get( uint8_t* data )
{
   *data = currentValue;
   return 1;
}

void HmwBrightnessSwitch::set( uint8_t length, uint8_t const* const data )
{
	if (*data)
	{
		if ( count )	// at least one sample was captured, now we proceed
			triggered = true;
	}
	else
		triggered = false;
}

void HmwBrightnessSwitch::loop( uint8_t channel )
{
   // start next measurement after x seconds (min. 10s (larger than analogIn sample interval), up to 255 seconds)
   nextActionDelay = config->interval* 1000;
   
   if ( !nextActionDelay )	// not used value, return >100%
   {
	  currentValue = 255;
      return;
   }

   if ( lastActionTime.since() < nextActionDelay *10 )
   {
      return;
   }

   lastActionTime = Timestamp();

   uint8_t reading;
   reading = linkedAnalogChannel->currentValue / 20;
   if (reading > 200) reading = 200;	// limit to 100% (200 / 2 = 100%)
	
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
}
