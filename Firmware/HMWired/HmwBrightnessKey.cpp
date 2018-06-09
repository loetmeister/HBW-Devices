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
	if ( linkedBrightnessSwitchChannel->triggered )
	{
		if ( (compareConditionLE() && linkedBrightnessSwitchChannel->currentValue <= config->triggerLevel)
			|| (!compareConditionLE() && linkedBrightnessSwitchChannel->currentValue >= config->triggerLevel) )
		{
			// if return value is 1, bus is not idle, retry next time
			if ( HmwDevice::sendKeyEvent( channel, keyPressNum, false ) == Stream::SUCCESS )
			{
				keyPressNum++;
				linkedBrightnessSwitchChannel->triggered = false;
			}
		}
		else
		{
			linkedBrightnessSwitchChannel->triggered = false;	// discard trigger - triggerLevel not matching
		}
	}
}

void HmwBrightnessKey::resetChannel()
{
   keyPressNum = 0;
}

void HmwBrightnessKey::checkConfig()
{
	if ( config->triggerLevel == 255 )
		config->triggerLevel = DEFAULT_TRIGGER_LEVEL;
   resetChannel();
}
