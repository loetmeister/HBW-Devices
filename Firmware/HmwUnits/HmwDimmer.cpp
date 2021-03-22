/*
 * HBWDimmer.cpp
 *
 * Created: 26.04.2017 09:01:56
 * Author: viktor.pankraz
 */


#include "HmwDimmer.h"
#include "HmwDevice.h"

#define getId() FSTR( "HmwDimmer." ) << channelId

const uint8_t HmwDimmer::debugLevel( DEBUG_LEVEL_OFF ); // DEBUG_LEVEL_LOW | DEBUG_STATE_L3 );

HmwDimmer::HmwDimmer( PortPin _portPin, PortPin _enablePin, Config* _config, uint8_t _normalizeLevel ) :
   normalizeLevel( _normalizeLevel ),
   dimmingFactor( _normalizeLevel ),
   dimmingOffset( 0 ),
   pwmOutput( _portPin.getPortNumber(), _portPin.getPinNumber() ),
   enableOutput( _enablePin ),
   currentLevel( 0xFF ),
   config( _config ),
   actionParameter( NULL ),
   timeMode( TIME_MODE_ABSOLUTE ),
   lastKeyNum( 255 ),
   dimTargetLevel( 0 )
{
   type = HmwChannel::HMW_DIMMER;
   pwmOutput.setInverted( config->isDimmingModeLeading() );
   setLevel( 0, false );
   SET_STATE_L1( JT_OFF );
}


void HmwDimmer::set( uint8_t length, uint8_t const* const data )
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

      DEBUG_H2( FSTR( "setAction 0x" ), (uintptr_t)actionParameter );
      DEBUG_M2( FSTR( "actionType   " ), actionParameter->actionType );
      DEBUG_M2( FSTR( "multiExecute " ), actionParameter->multiExecute );
      DEBUG_M2( FSTR( "onDelayMode  " ), actionParameter->onDelayMode );
      DEBUG_M2( FSTR( "offTimeMode  " ), actionParameter->offTimeMode );
      DEBUG_M2( FSTR( "onTimeMode   " ), actionParameter->onTimeMode );
      DEBUG_M2( FSTR( "offLevel     " ), actionParameter->offLevel );
      DEBUG_M2( FSTR( "onMinLevel   " ), actionParameter->onMinLevel );
      DEBUG_M2( FSTR( "onLevel      " ), actionParameter->onLevel );
      DEBUG_M2( FSTR( "rampStartStep" ), actionParameter->rampStartStep );
      DEBUG_M2( FSTR( "offDelayStep " ), actionParameter->offDelayStep );
      DEBUG_M2( FSTR( "onDelayTime  " ), actionParameter->onDelayTime );
      DEBUG_M2( FSTR( "rampOnTime   " ), actionParameter->rampOnTime );
      DEBUG_M2( FSTR( "onTime       " ), actionParameter->onTime );
      DEBUG_M2( FSTR( "offDelayTime " ), actionParameter->offDelayTime );
      DEBUG_M2( FSTR( "rampOffTime  " ), actionParameter->rampOffTime );
      DEBUG_M2( FSTR( "offTime      " ), actionParameter->offTime );
      DEBUG_M2( FSTR( "dimMinLevel  " ), actionParameter->dimMinLevel );
      DEBUG_M2( FSTR( "dimMaxLevel  " ), actionParameter->dimMaxLevel );
      DEBUG_M2( FSTR( "dimStep      " ), actionParameter->dimStep );
      DEBUG_M2( FSTR( "jtOnDelay    " ), actionParameter->jtOnDelay );
      DEBUG_M2( FSTR( "jtRampOn     " ), actionParameter->jtRampOn );
      DEBUG_M2( FSTR( "jtOn         " ), actionParameter->jtOn );
      DEBUG_M2( FSTR( "jtOffDelay   " ), actionParameter->jtOffDelay );
      DEBUG_M2( FSTR( "jtRampOff    " ), actionParameter->jtRampOff );
      DEBUG_M2( FSTR( "jtOff        " ), actionParameter->jtOff );

      if ( actionParameter->actionType > JUMP_TO_TARGET )
      {
         // First key press goes here, repeated press only when LONG_MULTIEXECUTE is enabled
         if ( ( lastKeyNum != cmd->keyNum ) || ( ( lastKeyNum == cmd->keyNum ) && actionParameter->multiExecute ) )
         {
            uint8_t level = -1;
            switch ( actionParameter->actionType )
            {
               case TOGGLE_TO_COUNTER:
               {
                  level = ( cmd->keyNum & 1 ) ? actionParameter->onLevel : actionParameter->offLevel; // switch ON at odd numbers, OFF at even numbers
                  break;
               }
               case TOGGLE_INVERS_TO_COUNTER:
               {
                  level = ( cmd->keyNum & 1 ) ? actionParameter->offLevel : actionParameter->onLevel; // switch OFF at odd numbers, ON at even numbers
                  break;
               }
               case UPDIM:
               {
                  level = getNextDimLevel( true );
                  break;
               }
               case DOWNDIM:
               {
                  level = getNextDimLevel( false );
                  break;
               }
               case TOGGLEDIM:
               case TOGGLEDIM_TO_COUNTER:
               {
                  level = getNextDimLevel( cmd->keyNum & 1 );
                  break;
               }
               case TOGGLEDIM_INVERS_TO_COUNTER:
               {
                  level = getNextDimLevel( !( cmd->keyNum & 1 ) );
                  break;
               }
               default:
               {
                  WARN_3( FSTR( "HmwDimmer::set actionType: 0x" ), (uint8_t)actionParameter->actionType, FSTR( " not implemented" ) );
               }
            }
            DEBUG_M1( FSTR( "targetLevel: 0x" ) << level );

            if ( level <= MAX_LEVEL )
            {
               if ( actionParameter->actionType < UPDIM )
               {
                  // no ramp needed, set new level and stop statechart
                  setLevel( level );
                  nextActionTime.reset();
                  if ( level )
                  {
                     SET_STATE_L1( JT_ON );
                  }
                  else
                  {
                     SET_STATE_L1( JT_OFF );
                  }
               }
               else
               {
                  rampStep = 1;
                  nextStepTime = MAX_DIMMING_TIME / abs( currentLevel - level );
                  dimTargetLevel = level;
                  enable();
                  nextState = ( dimTargetLevel > currentLevel ) ? DIM_UP : DIM_DOWN;
                  DEBUG_M1( FSTR( "nextStepTime  : 0x" ) << nextStepTime );
                  handleStateChart();
               }
            }
         }
      }
      else if ( lastKeyNum == cmd->keyNum && !actionParameter->multiExecute )
      {
         // repeated key event for ACTION_TYPE == 1 (ACTION_TYPE == 0 already filtered by receiveKeyEvent, HBWLinkReceiver)
         // must be long press, but LONG_MULTIEXECUTE not enabled
      }
      else if ( actionParameter->actionType == JUMP_TO_TARGET )
      {
         if ( !nextActionTime.isValid()
            || isRampRunning()
            || isMinimalTimeMode() )
         {
            prepareNextState( true );
         }
         handleStateChart();
      }
      lastKeyNum = cmd->keyNum;  // store key press number, to identify repeated key events
   }
   else
   {
      ERROR_2( FSTR( "HmwDimmer::set unknown command length: 0x" ), length );
   }
}

uint8_t HmwDimmer::getNextDimLevel( bool dimUp )
{

   int16_t level = currentLevel;  // keep current level if we cannot dim up or down anymore

   if ( dimUp )
   {
      level += ( actionParameter->dimStep );
   }
   else
   {
      level -= ( actionParameter->dimStep );
   }
   // check limits
   if ( level >= actionParameter->dimMaxLevel )
   {
      return actionParameter->dimMaxLevel;
   }
   if ( level <= actionParameter->dimMinLevel )
   {
      return actionParameter->dimMinLevel;
   }
   // return calculated value
   return (uint8_t)level;
}


uint8_t HmwDimmer::get( uint8_t* data )
{
   StateFlags stateFlags;
   stateFlags.byte = 0;
   stateFlags.flags.working = isWorkingState();

   if ( isRampRunning() )
   {
      stateFlags.flags.upDown = ( currentState == JT_RAMP_ON ) ? 1 : 2;
   }

   // map 0-100% to 0-200
   *data++ = currentLevel;
   *data = stateFlags.byte;
   return 2;
}

void HmwDimmer::loop()
{
   if ( isNextActionPending() )
   {
      handleStateChart();
   }
   handleFeedback();
}

void HmwDimmer::checkConfig()
{
   config->checkOrRestore();
   pwmOutput.setInverted( config->isDimmingModeLeading() );
   calculateDimmingParameter();
}

void HmwDimmer::setLevel( uint8_t level, bool withLogging )
{
   if ( ( currentLevel != level ) )
   {
      DEBUG_H2( FSTR( " setLevel 0x" ), level );
      level ? enableOutput.set() : enableOutput.clear();
      currentLevel = level;

      // handle first all the special cases, where PWM is not needed
      // dimming mode PWM is currently not supported and will be handled like SWITCH mode
      if ( config->isDimmingModeSwitch() || config->isDimmingModePwm() )
      {
         // disable PWM in this mode, set only digital pin
         level ? pwmOutput.DigitalOutput::set() : pwmOutput.clear();
      }
      else
      {
         if ( config->isDimmingModeLeading() )
         {
            level = MAX_LEVEL - level;
         }
         if ( level )
         {
            uint16_t pwmValue = level * dimmingFactor + dimmingOffset;
            pwmOutput.set( pwmValue );
            DEBUG_M2( FSTR( " pwmValue 0x" ), pwmValue );
         }
         else
         {
            pwmOutput.clear();
         }
      }

      checkLogging( withLogging && config->isLogging() );
   }
}


void HmwDimmer::handleStateChart()
{
   DEBUG_H1( FSTR( ".handleStatechart()" ) );
   if ( actionParameter == NULL )
   {
      DEBUG_M1( FSTR( "no actionParameter" ) );
      nextActionTime.reset();
      return;
   }
   if ( handleSpecificState( nextState ) )
   {
      prepareNextState();
   }
}

bool HmwDimmer::handleSpecificState( uint8_t _state )
{
   // enter the state and execute action
   switch ( _state )
   {
      case JT_OFF:
      {
         SET_STATE_L1( JT_OFF );
         setLevel( actionParameter->offLevel );
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
         nextActionTime += nextStepTime;
         if ( (uint16_t)( currentLevel + rampStep ) < actionParameter->onLevel )
         {
            setLevel( currentLevel + rampStep, false );
            return false; // we need more time
         }
         break;
      }
      case JT_ON:
      {
         SET_STATE_L1( JT_ON );
         setLevel( actionParameter->onLevel );
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
         nextActionTime += nextStepTime;
         if ( ( currentLevel >= rampStep ) && ( ( currentLevel - rampStep ) >= actionParameter->offLevel ) )
         {
            setLevel( currentLevel - rampStep, false );
            return false; // we need more time
         }
         break;
      }
      case DIM_UP:
      {
         SET_STATE_L1( DIM_UP );
         nextActionTime += nextStepTime;
         if ( (uint16_t)( currentLevel + rampStep ) <= dimTargetLevel )
         {
            setLevel( currentLevel + rampStep, false );
            return false; // we need more time
         }
         break;
      }
      case DIM_DOWN:
      {
         SET_STATE_L1( DIM_DOWN );
         nextActionTime += nextStepTime;
         if ( ( currentLevel >= rampStep ) && ( ( currentLevel - rampStep ) >= dimTargetLevel ) )
         {
            setLevel( currentLevel - rampStep, false );
            return false; // we need more time
         }
         break;
      }
      default:
      {
         WARN_3( FSTR( "HmwSwitch::handleJumpToTargetCmd to state: 0x" ), nextState, FSTR( " not implemented" ) );
      }
   }
   return true;
}

void HmwDimmer::prepareNextState( bool fromAction )
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
         targetState = fromAction ? actionParameter->jtOnDelay : JT_RAMP_ON;
         delayTime = actionParameter->onDelayTime;
         break;
      }
      case JT_RAMP_ON:
      {
         DEBUG_M1( FSTR( "currentState is JT_RAMP_ON" ) );
         targetState = fromAction ? actionParameter->jtRampOn : JT_ON;
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
         targetState = fromAction ? actionParameter->jtOffDelay : JT_RAMP_OFF;
         delayTime = actionParameter->offDelayTime;
         break;
      }
      case JT_RAMP_OFF:
      {
         DEBUG_M1( FSTR( "currentState is JT_RAMP_OFF" ) );
         targetState = fromAction ? actionParameter->jtRampOff : JT_OFF;
         break;
      }
      case DIM_UP:
      case DIM_DOWN:
      {
         DEBUG_M1( FSTR( "currentState is DIM_UP/DOWN" ) );
         if ( currentLevel )
         {
            SET_STATE_L1( JT_ON );
         }
         else
         {
            SET_STATE_L1( JT_OFF );
         }
         checkLogging( config->isLogging() );
         nextActionTime.reset();
         return;
      }
      default:
      {
         WARN_1( FSTR( "HmwDimmer::prepareNextState() invalid current state: 0x" ) << currentState );
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
      WARN_2( FSTR( "HmwSwitch::prepareNextState() has no transition to state : " ), targetState );
      nextActionTime.reset();
   }

   calculateRampParameter( targetState );

   DEBUG_M1( FSTR( "nextActionDelay : " ) << delayTime );
   DEBUG_M1( FSTR( "nextState       : " ) << nextState );
   DEBUG_M1( FSTR( "nextActionTime  : " ) << nextActionTime.get() );
}

void HmwDimmer::calculateDimmingParameter()
{
   DEBUG_H1( FSTR( " calculateDimmingParameter" ) );

   uint8_t cutOffStart = Config::_00_PERCENT - config->getPwmRangeStart();
   uint8_t cutOffEnd = Config::_100_PERCENT - config->getPwmRangeEnd();

   dimmingOffset = normalizeLevel * ( MAX_LEVEL / 10 );
   if ( config->isDimmingModeLeading() )
   {
      dimmingOffset *= cutOffEnd;
   }
   else
   {
      dimmingOffset *= cutOffStart;
   }

   // use extra 16bit variable to have accurate calculation for the dimmingFactor
   uint16_t pwmRange = normalizeLevel * MAX_LEVEL;
   pwmRange = pwmRange - ( normalizeLevel * ( cutOffStart + cutOffEnd ) * ( MAX_LEVEL / 10 ) );

   dimmingFactor = pwmRange / MAX_LEVEL;

   DEBUG_M2( FSTR( "dimmingOffset : 0x" ), dimmingOffset );
   DEBUG_M2( FSTR( "dimmingFactor : 0x" ), dimmingFactor );
}

void HmwDimmer::calculateRampParameter( uint8_t targetState )
{
   // only calculate ramp parameters if is needed for next state
   if ( ( targetState != JT_RAMP_ON ) && ( targetState != JT_RAMP_OFF ) )
   {
      return;
   }
   DEBUG_H1( FSTR( " calculateRampParameter" ) );
   rampStep = actionParameter->onLevel - actionParameter->offLevel;
   nextStepTime = actionParameter->rampOnTime;

   if ( targetState == JT_RAMP_OFF )
   {
      nextStepTime = actionParameter->rampOffTime;
   }
   nextStepTime = nextStepTime * SystemTime::S / 10;

   DEBUG_M2( FSTR( "total Steps : 0x" ), rampStep );
   DEBUG_M2( FSTR( "total Time : 0x" ), nextStepTime );

   if ( nextStepTime >= ( rampStep * LOOP_PERIOD_MS ) )
   {
      nextStepTime /= rampStep;
      rampStep = 1;
   }
   else
   {
      while ( ( nextStepTime >= ( 2 * LOOP_PERIOD_MS ) ) && ( rampStep >= actionParameter->dimStep ) )
      {
         rampStep /= 2;
         nextStepTime /= 2;
      }
   }

   DEBUG_M2( FSTR( "rampStep : 0x" ), rampStep );
   DEBUG_M2( FSTR( "stepTime : 0x" ), nextStepTime );
}
