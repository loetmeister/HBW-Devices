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
   linkedAnalogChannel( &_linkedAnalogChannel ),
   lastActionTime( 0 ),
   //actionParameter( NULL ),
   result_sum( 0 ),
   index( 0 ),
   count( 0 )
{
   //nextActionDelay = 1800;	// don't start before analog sampling (->HmwAnalogIn)
   currentValue = 0;
   triggered = false;
   stateFlags.byte = 0;
}


// allow to read the calculated level (brightness value 0...100%)
uint8_t HmwBrightnessSwitch::get( uint8_t* data )
{
   *data++ = currentValue;
   *data = stateFlags.byte;
   return 2;
}
	
void HmwBrightnessSwitch::set( uint8_t length, uint8_t const* const data )
{
	// will be set by peered key channel (motion sensor)
	
	//if ( length == sizeof( LinkCommand ) )  //TOD: ... HBWLinkLed works differently to dimmer...
	if ( count && length >= 6 )
	{
		//LinkCommand const* cmd = (LinkCommand const*)data;
		//actionParameter = cmd->actionParameter;
		
		triggered = false;
		stateFlags.element.active = false;
		
		switch ( data[0] )
		{
			case HmwChannel::TOGGLE:	// X LE COND_VALUE
				if ( currentValue <= data[2] )//actionParameter->onLevel )
				{
					triggered = true;
					stateFlags.element.le = true;
				}
				break;
			case HmwChannel::BLINK_ON:	// X GE COND_VALUE
				if ( currentValue >= data[2] )//actionParameter->onLevel )
				{
					triggered = true;
					stateFlags.element.le = false;
				}
				break;
			//case HmwChannel::BLINK_TOGGLE:	// not used
				//break;
		}
		
		if ( triggered )
		{
			stateFlags.element.active = true;
			
			if ( data[5] )//actionParameter->blockingTime )	//TODO: block re-trigger? or just pause brightness calculation??
			{
				nextActionDelay = data[5] *100;//actionParameter->blockingTime *100;
				stateFlags.element.blockingTimeActive = true;
			}
			else {
				stateFlags.element.blockingTimeActive = false;
			}
		}
	}
}

void HmwBrightnessSwitch::loop( uint8_t channel )
{
   if ( !nextActionDelay )	// not used value, returns >100% (TODO: test if this is good solution...)
   {
	  currentValue = 255;
      return;
   }

   if ( lastActionTime.since() < nextActionDelay *10 )
   {
      return;
   }

   lastActionTime = Timestamp();
   
   if ( stateFlags.element.blockingTimeActive )
   {
	   stateFlags.element.blockingTimeActive = false;
	   nextActionDelay = config->interval* 1000;	// restore configured value
   }
   
// TODO: add option to block channel for some seconds? to not send high bigness and allow re-trigger?

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

void HmwBrightnessSwitch::checkConfig()
{
   // start next calculation after x seconds
   // min. 10s (larger than analogIn sample interval), up to 45 seconds (gives 1 hour with 8 samples)
   nextActionDelay = config->interval* 1000;
   stateFlags.byte = 0;
}