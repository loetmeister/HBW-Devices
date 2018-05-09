/*
 * HmwAnalogIn.cpp
 *
 *  Created on: 26.04.2017
 *      Author: Viktor Pankraz
 */

#include "HmwAnalogIn.h"
#include "HmwDevice.h"
#include "Peripherals/Adc.h"


#define ADC_CH0      (1U << 0)                 /**< ADC channel 0. */
#define ADC_CH1      (1U << 1)                 /**< ADC channel 1. */

HmwAnalogIn::HmwAnalogIn( ADC_t* _adc, uint8_t _adcInputPin, Config* _config ) :
   adc( _adc ),
   adcInputPin( _adcInputPin ),
   config( _config ),
   state( INIT_ADC )
{
   nextActionDelay = 3300;	// some start delay
   lastActionTime = 0;
   currentValue = 0;
}


uint8_t HmwAnalogIn::get( uint8_t* data )
{
   // MSB first
   *data++ = ( currentValue >> 8 ) & 0xFF;
   *data = currentValue & 0xFF;
   return 2;
}

void HmwAnalogIn::loop( uint8_t channel )
{
   if ( !nextActionDelay )
   {
      return;
   }

   if ( lastActionTime.since() < nextActionDelay )
   {
      return;
   }

   lastActionTime = Timestamp();   // at least last time of trying
   
   // select current ADC module and result channel
   Adc& adc = Adc::instance( PortA );
   Adc::Channel& AdcChannel = adc.getChannel( adcInputPin );
   
   uint8_t adcChannelMask = ADC_CH0;
   if ( adcInputPin == 7)	// we store ADC pin6 and pin7 results in channel 0 and 1
   {
	   adcChannelMask = ADC_CH1;
   }
   
   if ( state == INIT_ADC )
   {
	   // based on example from http://wa4bvy.com/xmega_doxy/adc_quickstart.html

	   adc.setConversionParameter( false, ADC_RESOLUTION_12BIT_gc, ADC_REFSEL_INT1V_gc );
	   adc.setConversionTrigger( ADC_SWEEP_01_gc, ADC_EVSEL_0123_gc, ADC_EVACT_NONE_gc );
	   adc.setClockRate( 200000UL );

	   AdcChannel.setupInput( ADC_CH_INPUTMODE_SINGLEENDED_gc, adcInputPin, false, ADC_CH_GAIN_1X_gc );

	   state = START_MEASUREMENT;
   }
   else if ( state == START_MEASUREMENT )
   {
      adc.enable();
      AdcChannel.startConversion();
//      nextActionDelay = 1;
      nextActionDelay = 21;
      state = SAMPLE_VALUES;
   }
   else if ( state == SAMPLE_VALUES )
   {
      static const uint8_t MAX_SAMPLES = 8;
      static uint16_t buffer[MAX_SAMPLES] = { 0, 0, 0, 0, 0, 0, 0, 0 };
      static uint8_t nextIndex = 0;

      if ( adc.getInterrupts( adcChannelMask ) )
      {
		 adc.clearInterrupts( adcChannelMask );
		 buffer[nextIndex++] =  AdcChannel.getResult();
         state = START_MEASUREMENT;

         if ( nextIndex >= MAX_SAMPLES )
         {
            nextIndex = 0;
            uint32_t sum = 0;
            uint8_t i = MAX_SAMPLES;
            do
            {
               sum += buffer[--i];
            }
            while ( i );
            currentValue = sum / MAX_SAMPLES;
            state = SEND_FEEDBACK;
         }
      }
   }
   else if ( state == SEND_FEEDBACK )
   {
      bool doSend = true;

      // do not send before min interval
      doSend &= !( config->minInterval && ( lastSentTime.since() < ( (uint32_t)config->minInterval * 1000 ) ) );
      doSend &= ( ( config->maxInterval && ( lastSentTime.since() >= ( (uint32_t)config->maxInterval * 1000 ) ) )
                || ( config->minDelta && ( (uint16_t)abs( currentValue - lastSentValue ) >= ( (uint16_t)config->minDelta * 10 ) ) ) );

      if ( doSend )
      {
         uint8_t data[2];
         uint8_t errcode = HmwDevice::sendInfoMessage( channel, get( data ), data );

         // sendInfoMessage returns 0 on success, 1 if bus busy, 2 if failed
         if ( errcode )
         {
            // bus busy
            // try again later, but insert a small delay
            nextActionDelay = 250;
            return;
         }
         lastSentValue = currentValue;
         lastSentTime = Timestamp();

      }
      // start next measurement after 1s
      state = START_MEASUREMENT;
      nextActionDelay = 1000;

   }
}
