/*
 * HmwBrightness.cpp
 *
 *  Created on: 07.06.2018
 *      Author: loetmeister.de
 */

#include "HmwBrightnessKey.h"
#include "HmwDevice.h"
#include "HmwBrightnessSwitch.h"


HmwBrightnessKey::HmwBrightnessKey( HmwBrightnessSwitch& _linkedBrightnessSwitchChannel, Config* _config ) :
   config( _config ),
   linkedBrightnessSwitchChannel( &_linkedBrightnessSwitchChannel )
{
   resetChannel();
}


void HmwBrightnessKey::loop( uint8_t channel )
{
	if ( linkedBrightnessSwitchChannel->currentValue == 255 )
		return;	// channel "not used" value (disabled) - so quit
	
	// every transition from not triggered to triggered will cause a key event
	if ( linkedBrightnessSwitchChannel->triggered )
	{
		// if return value is 1, bus is not idle, retry next time
		if ( HmwDevice::sendKeyEvent( channel, keyPressNum, false ) == IStream::SUCCESS )	//TODO?: allow short & long press? = (triggered >> 1)
		//if ( HmwDevice::sendKeyEvent( channel, keyPressNum, (linkedBrightnessSwitchChannel->triggered >>1) ) == IStream::SUCCESS )
		{
			keyPressNum++;
			linkedBrightnessSwitchChannel->triggered = false;
		}
	}
}

void HmwBrightnessKey::resetChannel()
{
   keyPressNum = 0;
   linkedBrightnessSwitchChannel->triggered = false;
}

void HmwBrightnessKey::checkConfig()
{
	//if ( config->triggerLevel == 255 )
		//config->triggerLevel = DEFAULT_TRIGGER_LEVEL;
   
   resetChannel();
}
