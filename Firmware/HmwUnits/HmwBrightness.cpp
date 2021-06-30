/*
 * HmwBrightness.cpp
 *
 * Created: 17.07.2018 21:04:23
 * Author: Viktor Pankraz
 */


#include "HmwBrightness.h"
#include "HmwDevice.h"

#include <Tracing/Logger.h>
#include <stdlib.h>

const HmwBrightness::MeasurementRangeParameter HmwBrightness::rangeParams[HmwBrightness::MAX_RANGE_PARAMETER] =
{
   {
      TC_CLKSEL_DIV4_gc,
      1,
      2
   },
   {
      TC_CLKSEL_DIV64_gc,
      16,
      30
   },
   {
      TC_CLKSEL_DIV1024_gc,
      256,
      500
   }
};

#define getId() FSTR( "HmwBrightness " )


const uint8_t HmwBrightness::debugLevel( DEBUG_LEVEL_LOW );

HmwBrightness::HmwBrightness( PortPin _portPin, TimerCounterChannel _tcChannel, Config* _config ) :
   measurePin( _portPin ),
   tcChannel( _tcChannel ),
   rangeParamsSet( _8MHZ ),
   config( _config ),
   startCount( 0 ),
   currentValue( 0 ),
   lastSentValue( 0 ),
   filterHelper( 0 )
{
   SET_STATE_L1( START_MEASUREMENT );
   measurePin.set();
   prepareNextMeasurement();
}

uint8_t HmwBrightness::get( uint8_t* data )
{
   // MSB first
   *data++ = HBYTE( currentValue );
   *data++ = LBYTE( currentValue );

   return 2;
}

void HmwBrightness::prepareNextMeasurement()
{
   tcChannel.disable();
   measurePin.configOutput();
   SET_STATE_L1( START_MEASUREMENT );

   // give some time to load capacitor for next measurement
   enable( MIN_MEASURE_DELAY );
}

void HmwBrightness::notifyNewMeasuredValue( uint16_t lux )
{
   // check if filter has to be initialized with first value
   if ( filterHelper == 0 )
   {
      filterHelper = ( lux << FILTER_LEVEL );
   }
   else
   {
      // filter new input values
      filterHelper += lux - currentValue;
   }
   currentValue = ( filterHelper >> FILTER_LEVEL );

   DEBUG_H3( lux, '\t', currentValue );
   tcChannel.disable();
   measurePin.configOutput();
   SET_STATE_L1( SEND_FEEDBACK );
}

void HmwBrightness::loop()
{
   if ( isDisabled() || !isNextActionPending() )
   {
      return;
   }

   if ( getCurrentState() == START_MEASUREMENT )
   {
      CriticalSection doNotInterrupt;
      tcChannel.getTimerCounter()->configClockSource( rangeParams[rangeParamsSet].clkSelection );
      tcChannel.clearCCFlag();
      startCount = tcChannel.getCurrentCount();
      tcChannel.enable();
      measurePin.configInput();
      enable( 1 );
      lastMeasurementStartTime.setNow();
      SET_STATE_L1( WAIT_MEASUREMENT_RESULT );
   }
   else if ( getCurrentState() == WAIT_MEASUREMENT_RESULT )
   {
      if ( lastMeasurementStartTime.since() > rangeParams[rangeParamsSet].measurementTimeout )
      {
         // this is handled as a timeout, switch range if possible and start new measurement
         if ( rangeParamsSet < ( MAX_RANGE_PARAMETER - 1 ) )
         {
            rangeParamsSet++;
            prepareNextMeasurement();
         }
         else
         {
            // its too dark and measuring takes too long, so it is anyway about 0 lx
            notifyNewMeasuredValue( 0 );
         }
      }
      else
      {
         // check for measurement result each loop
         if ( tcChannel.getCCFlag() )
         {
            uint16_t delta = tcChannel.getCapture() - startCount;

            if ( ( delta < SWITCH_RANGE_THRESHOLD ) && ( rangeParamsSet > 0 ) )
            {
               // switch into higher resolution and measure again
               rangeParamsSet--;
               prepareNextMeasurement();
               return;
            }

            // make sure that result will be uint32_t (depends on type of first operand)
            uint32_t us = (uint32_t)delta * rangeParams[rangeParamsSet].eighthMicroSecondPerTick;
            double potency = -1 - (double)config->potency / 1000;
            notifyNewMeasuredValue( pow( us / 100, potency ) * config->factor );

            if ( IS_DEBUG_LEVEL( DEBUG_LEVEL_LOW ) )
            {
               struct DebugData
               {
                  uint8_t paramset;
                  uint32_t us;
               } dd;

               dd.paramset = rangeParamsSet;
               dd.us = delta;
               HmwDevice::sendInfoMessage( channelId, sizeof( dd ), (const uint8_t*)&dd );
            }
         }
      }
   }

   if ( getCurrentState() == SEND_FEEDBACK )
   {
      bool doSend = perCentOf<uint32_t>( config->minDeltaPercent, lastSentValue ) <= (uint32_t)labs( currentValue - lastSentValue );

      if ( doSend && handleFeedback( SystemTime::S* config->minInterval ) )
      {
         lastSentValue = currentValue;
      }
      prepareNextMeasurement();
   }
}

void HmwBrightness::checkConfig()
{
   if ( config->minInterval && ( ( config->minInterval < 10 ) || ( config->minInterval > 3600 ) ) )
   {
      config->minInterval = 10;
   }
   if ( config->minDeltaPercent && ( ( config->minDeltaPercent < 10 ) || ( config->minDeltaPercent > 100 ) ) )
   {
      config->minDeltaPercent = 10;
   }
   if ( ( config->potency < 5 ) || ( config->potency > 200 ) )
   {
      config->potency = 120;
   }
   if ( ( config->factor < 5 ) || ( config->factor > 65500 ) )
   {
      config->factor = 36065;
   }

   nextFeedbackTime = SystemTime::now() + SystemTime::S* config->minInterval;
}