/*
 * HBWDimmer.cpp
 *
 * Created: 26.04.2017 09:01:56
 * Author: viktor.pankraz
 */


#include "HmwSwitch.h"
#include "HmwDevice.h"

#define getId() FSTR( "HmwSwitch." ) << channelId

const uint8_t HmwSwitch::debugLevel( DEBUG_LEVEL_OFF ); // DEBUG_LEVEL_LOW | DEBUG_STATE_L3 );

HmwSwitch::HmwSwitch( PortPin _portPin, Config* _config ) :
   output( _portPin ),
   currentLevel( 0xFF ),
   lastReportedLevel( 0xFF ),
   config( _config ),
   actionParameter( NULL ),
   lastKeyNum( 0 ),
   timeMode( TIME_MODE_ABSOLUTE )
{
   type = HmwChannel::HMW_SWITCH;
   SET_STATE_L1( JT_OFF );
   setLevel( 0, false );
}


void HmwSwitch::set( uint8_t length, uint8_t const* const data )
{
   if ( ( length == 1 ) && ( *data <= MAX_LEVEL ) )
   {
      setLevel( *data );
      nextActionTime.reset();
      if ( *data )
      {
         SET_STATE_L1( JT_ON );
      }
      else
      {
         SET_STATE_L1( JT_OFF );
      }
   }
   else if ( length == sizeof( LinkCommand ) )
   {
      LinkCommand const* cmd = (LinkCommand const*)data;
      actionParameter = cmd->actionParameter;

      if ( lastKeyNum == cmd->keyNum )
      {
         // repeated key event, must be long press
      }
      else if ( actionParameter->actionType == ACTIVE )
      {
         DEBUG_H2( FSTR( ".setAction 0x" ), (uintptr_t)actionParameter );
         DEBUG_M2( FSTR( "onTimeMode   " ), actionParameter->onTimeMode );
         DEBUG_M2( FSTR( "offTimeMode  " ), actionParameter->offTimeMode );
         DEBUG_M2( FSTR( "actionType   " ), actionParameter->actionType );
         DEBUG_M2( FSTR( "onDelayTime  " ), actionParameter->onDelayTime );
         DEBUG_M2( FSTR( "onTime       " ), actionParameter->onTime );
         DEBUG_M2( FSTR( "offDelayTime " ), actionParameter->offDelayTime );
         DEBUG_M2( FSTR( "offTime      " ), actionParameter->offTime );
         DEBUG_M2( FSTR( "jtOnDelay    " ), actionParameter->jtOnDelay );
         DEBUG_M2( FSTR( "jtOffDelay   " ), actionParameter->jtOffDelay );
         DEBUG_M2( FSTR( "jtOn         " ), actionParameter->jtOn );
         DEBUG_M2( FSTR( "jtOff        " ), actionParameter->jtOff );

         if ( !nextActionTime.isValid() || isMinimalTimeMode() )
         {
            prepareNextState( true );
         }
         handleStateChart();
      }
      lastKeyNum = cmd->keyNum;  // store key press number, to identify repeated key events
   }
   else
   {
      ERROR_2( FSTR( "HmwSwitch::set unknown command length: 0x" ), length );
   }
}

uint8_t HmwSwitch::get( uint8_t* data )
{
   StateFlags stateFlags;
   stateFlags.byte = 0;
   stateFlags.flags.working = isWorkingState();

   if ( currentLevel != lastReportedLevel )
   {
      if ( currentState == JT_ON )
      {
         stateFlags.flags.state = 1;
      }
      else if ( currentState == JT_OFF )
      {
         stateFlags.flags.state = 2;
      }
   }

   lastReportedLevel = currentLevel;
   *data++ = currentLevel;
   *data = stateFlags.byte;

   return 2;
}

void HmwSwitch::loop()
{
   if ( isNextActionPending() )
   {
      handleStateChart();
   }
   handleFeedback();
}

void HmwSwitch::checkConfig()
{
   config->checkOrRestore();
}

void HmwSwitch::setLevel( uint8_t level, bool withLogging )
{
   if ( currentLevel != level )
   {
      DEBUG_H2( FSTR( " setLevel 0x" ), level );
      level ? output.set() : output.clear();
      currentLevel = level;
      checkLogging( withLogging && config->isLogging() );
   }
}

void HmwSwitch::prepareNextState( bool fromAction )
{
   DEBUG_H1( FSTR( ".prepareNextState()" ) );
   uint8_t targetState = JT_OFF;
   TimeModes targetTimeMode = TIME_MODE_ABSOLUTE;
   uint16_t delayTime = 0;

   switch ( getCurrentState() )
   {
      case JT_OFF:
      {
         DEBUG_M1( FSTR( "currentState is JT_OFF" ) );
         targetState = fromAction ? actionParameter->jtOff : JT_ONDELAY;
         delayTime = actionParameter->offTime;
         targetTimeMode = (TimeModes)actionParameter->offTimeMode;
         break;
      }
      case JT_ONDELAY:
      {
         DEBUG_M1( FSTR( "currentState is JT_ONDELAY" ) );
         targetState = fromAction ? actionParameter->jtOnDelay : JT_ON;
         delayTime = actionParameter->onDelayTime;
         break;
      }
      case JT_ON:
      {
         DEBUG_M1( FSTR( "currentState is JT_ON" ) );
         targetState = fromAction ? actionParameter->jtOn : JT_OFFDELAY;
         delayTime = actionParameter->onTime;
         targetTimeMode = (TimeModes)actionParameter->onTimeMode;
         break;
      }
      case JT_OFFDELAY:
      {
         DEBUG_M1( FSTR( "currentState is JT_OFFDELAY" ) );
         targetState = fromAction ? actionParameter->jtOffDelay : JT_OFF;
         delayTime = actionParameter->offDelayTime;
         break;
      }
      default:
      {
         WARN_1( FSTR( "HmwSwitch::prepareNextState() invalid current state: 0x" ) << currentState );
      }
   }

   nextActionTime.reset();
   if ( isValidActionTime( delayTime ) )
   {
      nextActionTime = SystemTime::now() + convertToTime( delayTime );
   }

   if ( targetState <= JT_OFF )
   {
      nextState = targetState;
      timeMode = targetTimeMode;
   }
   else if ( targetState == JT_NO_JUMP_IGNORE_COMMAND )
   {
      // do nothing
   }
   else
   {
      WARN_2( FSTR( "HmwSwitch::prepareNextState() has no transition to state : " ), targetState );
      nextActionTime.reset();
   }

   DEBUG_M1( FSTR( "nextActionDelay : " ) << delayTime );
   DEBUG_M1( FSTR( "nextState       : " ) << nextState );
   DEBUG_M1( FSTR( "nextActionTime  : " ) << nextActionTime.get() );
}

void HmwSwitch::handleStateChart()
{
   DEBUG_H1( FSTR( ".handleStatechart()" ) );
   if ( actionParameter == NULL )
   {
      DEBUG_M1( FSTR( "no actionParameter" ) );
      nextActionTime.reset();
      return;
   }

   // enter next state and execute action
   switch ( nextState )
   {
      case JT_OFF:
      {
         SET_STATE_L1( JT_OFF );
         setLevel( 0 );
         break;
      }
      case JT_ONDELAY:
      {
         SET_STATE_L1( JT_ONDELAY );
         break;
      }
      case JT_ON:
      {
         SET_STATE_L1( JT_ON );
         setLevel( MAX_LEVEL );
         break;
      }
      case JT_OFFDELAY:
      {
         SET_STATE_L1( JT_OFFDELAY );
         break;
      }
      default:
      {
         WARN_3( FSTR( "HmwSwitch::handleJumpToTargetCmd to state: 0x" ), nextState, FSTR( " not implemented" ) );
      }
   }

   prepareNextState();
}
