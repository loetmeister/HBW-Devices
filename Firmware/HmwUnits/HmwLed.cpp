/*
 * HBWLed.cpp
 *
 * Created: 26.04.2017 09:01:56
 * Author: viktor.pankraz
 */


#include "HmwLed.h"
#include "HmwDevice.h"

#define getId() FSTR( "HmwLed." ) << channelId

const uint8_t HmwLed::debugLevel( DEBUG_LEVEL_OFF ); // DEBUG_LEVEL_LOW | DEBUG_STATE_L3 );

HmwLed::HmwLed( PortPin _portPin, Config* _config, bool _inverted, uint8_t _defaultPwmRange ) :
   pwmOutput( _portPin.getPortNumber(), _portPin.getPinNumber(), MAX_LEVEL_PERIOD ),
   defaultPwmRange( _defaultPwmRange )
{
   type = HmwChannel::HMW_LED;
   config = _config;
   pwmOutput.setInverted( _inverted );
   pwmOutput.DigitalOutput::clear();
   pwmOutput.clear();
   feedbackCmdActive = false;
   currentState = OFF;
   currentLevel = 0;
   onLevel = MAX_LEVEL;
   offLevel = 0;
   blinkOnTime = 10;
   blinkOffTime = 10;
   blinkQuantity = 255;
   lastKeyNum = 255;
   disable();
}


void HmwLed::set( uint8_t length, uint8_t const* const data )
{
   if ( *data <= MAX_LEVEL )
   {
      currentLevel = *data;
      disable();
      SET_STATE_L1( currentLevel ? ON : OFF );
   }
   else if ( isKeyFeedbackOnCmd( *data ) )
   {
      setLevel( MAX_LEVEL );
      feedbackCmdActive = true;
      return;   // no logging for feedbackCmd
   }
   else if ( isKeyFeedbackOffCmd( *data ) )
   {
      feedbackCmdActive = false;
      return;   // no logging for feedbackCmd
   }
   else if ( length >= 6 )
   {
      if ( lastKeyNum != data[6] )
      {
	     lastKeyNum = data[6];
         offLevel = data[1];
         onLevel = data[2]; // TODO add: onLevel = (data[2] > 200) ? currentLevel : data[2];  // special value 202 for current level
         blinkOnTime = data[3];
         blinkOffTime = data[4];
         blinkQuantity = data[5];
	     
         if ( isBlinkOnCmd( *data ) )
         {
            enable();
            SET_STATE_L1( BLINK_ON );
         }
         else if ( isBlinkToggleCmd( *data ) )
         {
            if ( currentState != BLINK_ON )
            {
               enable();
               SET_STATE_L1( BLINK_ON );
            }
            else
            {
               disable();
               SET_STATE_L1( currentLevel > offLevel ? ON : OFF );
            }
         }
		 else if ( isOnTimerCmd( *data ) )
		 {
			 if (blinkQuantity < 255) blinkQuantity += 1; // force blinkQuantity to step in to timer loop at least once
			 enable();
			 SET_STATE_L1( ON_TIMER );
		 }
         else if ( isToggleCmd( *data ) )
         {
            disable();
            if ( isLogicalOn() )
            {
               currentLevel = offLevel;
               SET_STATE_L1( OFF );
            }
            else
            {
               currentLevel = onLevel;
               SET_STATE_L1( ON );
            }
         }
	  }
   }
   else  // toggle
   {
      currentLevel = currentLevel ? 0 : MAX_LEVEL;
      disable();
   }

   checkLogging( config->isLogging() );
}


uint8_t HmwLed::get( uint8_t* data )
{
   StateFlags stateFlags;
   stateFlags.byte = 0;
   stateFlags.flags.state = getCurrentState() - OFF;
   stateFlags.flags.working = isWorkingState();

   // map 0-100% to 0-200
   *data++ = currentLevel;
   *data = stateFlags.byte;
   return 2;
}

void HmwLed::loop()
{
   if ( isNextActionPending() )
   {
	if ( getCurrentState() == ON_TIMER )  // stay on for blinkOnTime. If blinkQuantity is set, repeat onTime as many as blinkQuantity
	{
		if ( blinkQuantity )
		{
			nextActionTime += ( blinkOnTime * 100 );
			setLevel( onLevel );
			blinkQuantity--;
		}
		else
		{
			disable();
			SET_STATE_L1( currentLevel > offLevel ? ON : OFF );
			checkLogging( config->isLogging() );  // notify once blinking stopped (blinkQuantity == 0)
		}
	}
	else
	{
      // handle blinking
      if ( getLevel() == onLevel )
      {
         // is ON
         nextActionTime += ( blinkOffTime * 100 );
         setLevel( offLevel );
      }
      else
      {
         // is OFF
         if ( blinkQuantity )
         {
            nextActionTime += ( blinkOnTime * 100 );
            setLevel( onLevel );

            if ( blinkQuantity != 255 )
            {
               blinkQuantity--;
            }
         }
         else
         {
            disable();
            SET_STATE_L1( currentLevel > offLevel ? ON : OFF );
            checkLogging( config->isLogging() );  // notify once blinking stopped (blinkQuantity == 0)
         }
      }
	}
   }

   if ( !feedbackCmdActive )//&& !isWorkingState() )
   {
      if ( !isWorkingState() ) {
         // the default range is 0-200, this must be mapped to 0-100% duty cycle
         setLevel( currentLevel );
      }
	  else
	  {
		  if ( getCurrentState() == ON_TIMER )   setLevel( onLevel );   // go back to onLevel, when key press feedback is off
	  }
   }

   handleFeedback();
}

void HmwLed::checkConfig()
{
   if ( config->getPwmRange() > 100 )
   {
      config->setPwmRange( defaultPwmRange );
   }
}

void HmwLed::setLevel( uint8_t level )
{
   // special function for Config::levelFactor == 0, no PWM
   if ( config->getPwmRange() )
   {
      pwmOutput.set( level * config->getPwmRange() / NORMALIZE_LEVEL );
   }
   else
   {
      pwmOutput.set( level ? MAX_LEVEL_PERIOD : 0 );
   }
}

uint8_t HmwLed::getLevel() const
{
   // special function for Config::levelFactor == 0, no PWM
   if ( config->getPwmRange() )
   {
      return pwmOutput.isSet() * NORMALIZE_LEVEL / config->getPwmRange();
   }
   return pwmOutput.isSet() ? MAX_LEVEL : 0;
}