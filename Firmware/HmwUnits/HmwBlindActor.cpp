/*
 * HBWDimmer.cpp
 *
 * Created: 26.04.2017 09:01:56
 * Author: viktor.pankraz
 */


#include "HmwBlindActor.h"
#include "HmwDevice.h"

#define getId() FSTR( "HmwBlindActor." ) << channelId

const uint8_t HmwBlindActor::debugLevel( DEBUG_LEVEL_LOW | DEBUG_STATE_L3 );

HmwBlindActor::HmwBlindActor( PortPin _moveUp, PortPin _moveDown, Config* _config ) :
   moveUpOutput( _moveUp ),
   moveDownOutput( _moveDown ),
   currentLevel( 0 ),
   config( _config ),
   actionParameter( NULL ),
   timeMode( TIME_MODE_ABSOLUTE ),
   lastRepeatedKeyPressTime( 0 ),
   lastKeyNum( 0 ),
   targetLevel( 0 )
{
   type = HmwChannel::HMW_BLIND_ACTOR;
   moveDownOutput.clear();
   moveUpOutput.clear();
   SET_STATE_L1( JT_OFF );
}


void HmwBlindActor::set( uint8_t length, uint8_t const* const data )
{
   if ( length == 1 )
   {
      if ( *data <= MAX_LEVEL )
      {
         setTargetLevel( *data );
      }
      else if ( *data == BlindActorCommands::STOP )
      {
         stop();
      }
      actionParameter = NULL;
   }
   else if ( length == sizeof( LinkCommand ) )
   {
      LinkCommand const* cmd = (LinkCommand const*)data;
      actionParameter = cmd->actionParameter;
      /*
         DEBUG_H( FSTR( "setAction 0x" ), (uintptr_t)actionParameter );
         DEBUG_M( FSTR( "actionType   " ), actionParameter->actionType );
         DEBUG_M( FSTR( "toggleUse    " ), actionParameter->toggleUse );
         DEBUG_M( FSTR( "drivingMode  " ), actionParameter->drivingMode );
         DEBUG_M( FSTR( "offTimeMode  " ), actionParameter->offTimeMode );
         DEBUG_M( FSTR( "onTimeMode   " ), actionParameter->onTimeMode );
         DEBUG_M( FSTR( "offLevel     " ), actionParameter->offLevel );
         DEBUG_M( FSTR( "onLevel      " ), actionParameter->onLevel );
         DEBUG_M( FSTR( "onDelayTime  " ), actionParameter->onDelayTime );
         DEBUG_M( FSTR( "offDelayTime " ), actionParameter->offDelayTime );
         DEBUG_M( FSTR( "onTime       " ), actionParameter->onTime );
         DEBUG_M( FSTR( "offTime      " ), actionParameter->offTime );
         DEBUG_M( FSTR( "maxTimeFirst " ), actionParameter->maxTimeFirstDirection );
         DEBUG_M( FSTR( "jtOnDelay    " ), actionParameter->jtOnDelay );
         DEBUG_M( FSTR( "jtRefOn      " ), actionParameter->jtRefOn );
         DEBUG_M( FSTR( "jtRampOn     " ), actionParameter->jtRampOn );
         DEBUG_M( FSTR( "jtOn         " ), actionParameter->jtOn );
         DEBUG_M( FSTR( "jtOffDelay   " ), actionParameter->jtOffDelay );
         DEBUG_M( FSTR( "jtRefOff     " ), actionParameter->jtRefOff );
         DEBUG_M( FSTR( "jtRampOff    " ), actionParameter->jtRampOff );
         DEBUG_M( FSTR( "jtOff        " ), actionParameter->jtOff );
       */
      if ( lastKeyNum == cmd->keyNum )
      {
         lastRepeatedKeyPressTime = Timestamp();
      }
      else if ( actionParameter->actionType == ACTIVE )
      {
         if ( !nextActionTime.isValid()
            || isRampRunning()
            || isMinimalTimeMode() )
         {
            prepareNextState( true );
         }
         handleStateChart();

         targetLevel = ( currentState < JT_ON ) ? actionParameter->onLevel : actionParameter->offLevel;

         // special case where the actor is logically in an end position but needs to move 5% more into the same direction
         if ( !config->isKeepingLogicalEnd() && ( currentLevel == targetLevel ) )
         {
            if ( targetLevel == MAX_LEVEL )
            {
               setLevel( MAX_LEVEL - ( MAX_LEVEL / 100 * 5 ), false );
            }
            else if ( targetLevel == 0 )
            {
               setLevel( MAX_LEVEL / 100 * 5, false );
            }
         }
      }
      lastKeyNum = cmd->keyNum;  // store key press number, to identify repeated key events
   }
   else
   {
      LOG_ERROR( FSTR( "HmwBlindActor::set unknown command length: 0x" ) << length );
   }
}


uint8_t HmwBlindActor::get( uint8_t* data )
{
   StateFlags stateFlags;
   stateFlags.byte = 0;
   stateFlags.flags.working = isWorkingState();

   if ( isRampRunning() )
   {
      stateFlags.flags.upDown = isInRampOn() ? 1 : 2;
   }

   // map 0-100% to 0-200
   *data++ = currentLevel;
   *data = stateFlags.byte;
   return 2;
}

void HmwBlindActor::stop()
{
   uint8_t level = currentLevel;
   if ( isInRampOff() && ( level > 0 ) )
   {
      level--;
   }
   else if ( isInRampOn() && ( level < MAX_LEVEL ) )
   {
      level++;
   }
   setTargetLevel( level );
}

void HmwBlindActor::loop()
{
   if ( isNextActionPending() )
   {
      if ( lastRepeatedKeyPressTime.isValid() )
      {
         if ( lastRepeatedKeyPressTime.since() > REPEAT_LONG_PRESS_TIMEOUT )
         {
            lastRepeatedKeyPressTime.reset();
            stop();
         }
      }
      handleStateChart();
   }
   handleFeedback();
}

void HmwBlindActor::checkConfig()
{
   config->checkOrRestore();
   setLevel( config->lastPosition, true );
}

void HmwBlindActor::setLevel( uint8_t level, bool withLogging )
{
   if ( ( currentLevel != level ) )
   {
      DEBUG_H( FSTR( " setLevel 0x" ) << level );
      currentLevel = level;

      checkLogging( withLogging && config->isLogging() );
   }
}

void HmwBlindActor::setTargetLevel( uint8_t level )
{
   targetLevel = level;
   if ( !nextActionTime.isValid() && ( currentLevel != targetLevel ) )
   {
      // got new target, needs to start behavior
      nextState = ( targetLevel < currentLevel ) ? JT_OFFDELAY : JT_ONDELAY;
      enable();
   }
}

void HmwBlindActor::handleStateChart()
{
   DEBUG_H( FSTR( ".handleStatechart()" ) );
   if ( handleSpecificState( nextState ) )
   {
      prepareNextState();
   }
}

bool HmwBlindActor::handleSpecificState( uint8_t _state )
{
   // enter the state and execute action
   switch ( _state )
   {
      case JT_OFF:
      {
         SET_STATE_L1( JT_OFF );
         moveDownOutput.clear();
         moveUpOutput.clear();
         config->lastPosition.update( currentLevel );
         break;
      }
      case JT_ONDELAY:
      {
         SET_STATE_L1( JT_ONDELAY );
         break;
      }
      case JT_RAMP_ON:
      {
         SET_STATE_L1( JT_RAMP_ON );
         if ( !moveUpOutput.isSet() )
         {
            moveUpOutput.set();
         }
         else
         {
            setLevel( currentLevel + 1 );
         }
         if ( currentLevel < targetLevel )
         {
            nextActionTime += config->getRunTimeStepUp();
            return false; // we need more time
         }
         break;
      }
      case JT_ON:
      {
         SET_STATE_L1( JT_ON );
         moveDownOutput.clear();
         moveUpOutput.clear();
         config->lastPosition.update( currentLevel );
         break;
      }
      case JT_OFFDELAY:
      {
         SET_STATE_L1( JT_OFFDELAY );
         break;
      }
      case JT_RAMP_OFF:
      {
         SET_STATE_L1( JT_RAMP_OFF );
         if ( !moveDownOutput.isSet() )
         {
            moveDownOutput.set();
         }
         else
         {
            setLevel( currentLevel - 1 );
         }
         if ( currentLevel > targetLevel )
         {
            nextActionTime += config->getRunTimeStepDown();
            return false; // we need more time
         }
         break;
      }
      case JT_REF_ON:
      {
         SET_STATE_L1( JT_REF_ON );
         // TODO
         break;
      }
      case JT_REF_OFF:
      {
         SET_STATE_L1( JT_REF_OFF );
         // TODO
         break;
      }
      default:
      {
         LOG_WARNING( FSTR( "HmwSwitch::handleJumpToTargetCmd to state: 0x" ) << nextState << FSTR( " not implemented" ) );
      }
   }
   return true;
}

void HmwBlindActor::prepareNextState( bool fromAction )
{
   DEBUG_H( FSTR( ".prepareNextState()" ) );
   uint8_t targetState = JT_OFF;
   TimeModes targetTimeMode = TIME_MODE_ABSOLUTE;
   uint16_t delayTime = 0;

   switch ( getCurrentState() )
   {
      case JT_OFF:
      {
         DEBUG_M( FSTR( "currentState is JT_OFF" ) );
         targetState = fromAction ? actionParameter->jtOff : JT_ONDELAY;
         if ( actionParameter )
         {
            delayTime = actionParameter->offTime;
            targetTimeMode = (TimeModes)actionParameter->offTimeMode;
         }
         else
         {
            delayTime = ( targetLevel == currentLevel ) ? ENDLESS_TIME : config->getChangeOverDelayTime();
         }
         break;
      }
      case JT_ONDELAY:
      {
         DEBUG_M( FSTR( "currentState is JT_ONDELAY" ) );
         targetState = fromAction ? actionParameter->jtOnDelay : JT_RAMP_ON;
         if ( actionParameter )
         {
            delayTime = actionParameter->onDelayTime;
         }
         break;
      }
      case JT_RAMP_ON:
      {
         DEBUG_M( FSTR( "currentState is JT_RAMP_ON" ) );
         targetState = fromAction ? actionParameter->jtRampOn : JT_ON;
         break;
      }
      case JT_ON:
      {
         DEBUG_M( FSTR( "currentState is JT_ON" ) );
         targetState = fromAction ? actionParameter->jtOn : JT_OFFDELAY;
         if ( actionParameter )
         {
            delayTime = actionParameter->onTime;
            targetTimeMode = (TimeModes)actionParameter->onTimeMode;
         }
         else
         {
            delayTime = ( targetLevel == currentLevel ) ? ENDLESS_TIME : config->getChangeOverDelayTime();
         }
         break;
      }
      case JT_OFFDELAY:
      {
         DEBUG_M( FSTR( "currentState is JT_OFFDELAY" ) );
         targetState = fromAction ? actionParameter->jtOffDelay : JT_RAMP_OFF;
         if ( actionParameter )
         {
            delayTime = actionParameter->offDelayTime;
         }
         break;
      }
      case JT_RAMP_OFF:
      {
         DEBUG_M( FSTR( "currentState is JT_RAMP_OFF" ) );
         targetState = fromAction ? actionParameter->jtRampOff : JT_OFF;
         break;
      }
      default:
      {
         LOG_WARNING( FSTR( "HmwBlindActor::prepareNextState() invalid current state: 0x" ) << currentState );
      }
   }

   nextActionTime = SystemTime::now() + convertToTime( delayTime );
   if ( !isValidActionTime( delayTime ) )
   {
      nextActionTime.reset();
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
      LOG_WARNING( FSTR( "HmwSwitch::prepareNextState() has no transition to state : " ) << targetState );
      nextActionTime.reset();
   }

   DEBUG_M( FSTR( "nextActionDelay : " ) << delayTime );
   DEBUG_M( FSTR( "nextState       : " ) << nextState );
   DEBUG_M( FSTR( "nextActionTime  : " ) << nextActionTime.get() );
}

