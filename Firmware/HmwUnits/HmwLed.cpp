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
   lastKeyNum = -1;
   disable();
}


void HmwLed::set( uint8_t length, uint8_t const* const data )
{
   if ( *data <= MAX_LEVEL )
   {
      currentLevel = *data;
      disable();
      if ( *data )
      {
         SET_STATE_L1( ON );
      }
      else
      {
         SET_STATE_L1( OFF );
      }
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
      if ( lastKeyNum != data[6] ) {
	  lastKeyNum = data[6];
      offLevel = data[1];
      onLevel = data[2];
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
            SET_STATE_L1( OFF );
         }
      }
      else if ( isToggleCmd( *data ) )
      {
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
         disable();
      }
	  }
   }
   else  // toggle
   {
      if ( currentLevel )
      {
         currentLevel = 0;
      }
      else
      {
         currentLevel = MAX_LEVEL;
      }
      disable();
   }

   checkLogging( config->isLogging() );
}


uint8_t HmwLed::get( uint8_t* data )
{
   StateFlags stateFlags;
   stateFlags.byte = 0;
   stateFlags.flags.working = isWorkingState();
   stateFlags.flags.state = getCurrentState() - ON;

   // map 0-100% to 0-200
   *data++ = currentLevel;
   *data = stateFlags.byte;
   return 2;
}

void HmwLed::loop()
{
   if ( isNextActionPending() )
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
            if ( currentLevel == onLevel )
            {
               SET_STATE_L1( ON );
            }
            else
            {
               SET_STATE_L1( OFF );
            }
         }
      }
   }
   if ( !feedbackCmdActive && !isWorkingState() )
   {
      // the default range is 0-200, this must be mapped to 0-100% duty cycle
      setLevel( currentLevel );
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