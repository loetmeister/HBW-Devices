/*
 * HmwAnalogIn.cpp
 *
 *  Created on: 26.04.2017
 *      Author: Viktor Pankraz
 *  Changed on: 01.05.2022
 *      Author: loetmeister.de
 */

#include "HmwAnalogIn.h"
#include "HmwDevice.h"
#include "Peripherals/Adc.h"


#define getId() FSTR( "HmwAnalogIn " )

const uint8_t HmwAnalogIn::debugLevel( DEBUG_LEVEL_OFF );

#define ADC_CH0      (1U << 0)                 /**< ADC channel 0. */
#define ADC_CH1      (1U << 1)                 /**< ADC channel 1. */

#define SAMPLE_INTERVAL_ADC 380	 // 380 ms *4 samples
#define MEASUREMET_CYCLE 5000  // pause after all samples had been taken (and currentValue was updated)


HmwAnalogIn::HmwAnalogIn( uint8_t _adcInputPort, uint8_t _adcInputPin, Config* _config ) :
   adc( &ADCA ),	// fixed to Analog-to-Digital Converter 'A' (32a4u has only one ADC  anyway)
   adcInputPin( _adcInputPin ),
   adcInputPort( _adcInputPort ),
   config( _config ),
   lastSentValue ( 0 ),
   nextIndex ( 0 )
{
   SET_STATE_L1( INIT_ADC );
   enable ( MEASUREMET_CYCLE /2 );	// some start delay
   currentValue = 0;
}

uint8_t HmwAnalogIn::get( uint8_t* data )
{
   // MSB first
   *data++ = ( currentValue >> 8 );
   *data = currentValue & 0xFF;
   return 2;
}

void HmwAnalogIn::loop()
{
   if ( !isNextActionPending() )
   {
      return;
   }
   
   // select current ADC module and result channel
   Adc& adc = Adc::instance( adcInputPort );
   Adc::Channel& AdcChannel = adc.getChannel( adcInputPin );
   
   uint8_t adcChannelMask = ADC_CH0;
   if ( adcInputPin == 7)	// we store ADC pin6 and pin7 results in channel 0 and 1
   {
	   adcChannelMask = ADC_CH1;
   }
   
   if ( getCurrentState() == INIT_ADC )
   {
	   // based on example from http://wa4bvy.com/xmega_doxy/adc_quickstart.html

	   adc.setConversionParameter<false, ADC_RESOLUTION_12BIT_gc, ADC_REFSEL_INTVCC_gc>();	// unsigned, 12bit result, use "VCC / 1.6" ref. voltage
	   adc.setConversionTrigger<ADC_SWEEP_01_gc, ADC_EVSEL_0123_gc, ADC_EVACT_NONE_gc>();
	   adc.setClockRate<200000UL>();

	   AdcChannel.setupInput( ADC_CH_INPUTMODE_SINGLEENDED_gc, adcInputPin, false, ADC_CH_GAIN_1X_gc );	// input mode, posPin, negPin, gain

	   SET_STATE_L1( START_MEASUREMENT );
   }
   else if ( getCurrentState() == START_MEASUREMENT )
   {
      adc.enable();
      AdcChannel.startConversion();
	  enable( SAMPLE_INTERVAL_ADC );
      SET_STATE_L1( SAMPLE_VALUES );
   }
   else if ( getCurrentState() == SAMPLE_VALUES )
   {
      if ( adc.getInterrupts( adcChannelMask ) )
      {
		 adc.clearInterrupts( adcChannelMask );
		 buffer[nextIndex++] =  AdcChannel.getResult();
         SET_STATE_L1( START_MEASUREMENT );

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
            SET_STATE_L1( SEND_FEEDBACK );
         }
      }
   }
   else if ( getCurrentState() == SEND_FEEDBACK )
   {
      bool doSend = true;

      // do not send before min interval
	  doSend &= ( ( config->maxInterval && ( ( nextFeedbackTime.since() / SystemTime::S ) >= ( config->maxInterval - config->minInterval ) ) )
                || ( config->minDelta && ( (uint16_t)abs( currentValue - lastSentValue ) >= ( (uint16_t)config->minDelta * 10 ) ) ) );

      if ( doSend && handleFeedback( SystemTime::S* config->minInterval ) )
      {
         lastSentValue = currentValue;
	  }
	  
      // start next measurement
      SET_STATE_L1( START_MEASUREMENT );
	  enable( MEASUREMET_CYCLE );
   }
}

void HmwAnalogIn::checkConfig()
{
	if ( config->minDelta > 250 )
	{
		config->minDelta = 50;
	}
	if ( config->minInterval && ( ( config->minInterval < 5 ) || ( config->minInterval > 3600 ) ) )
	{
		config->minInterval = 10;
	}
	if ( config->maxInterval && ( ( config->maxInterval < 5 ) || ( config->maxInterval > 3600 ) ) )
	{
		config->maxInterval = 150;
	}
	if ( config->maxInterval && ( config->maxInterval < config->minInterval ) )
	{
		config->maxInterval = 0;
	}

	nextFeedbackTime = SystemTime::now() + SystemTime::S* config->minInterval;
	
	if ( !config->minDelta && !config->minInterval && !config->maxInterval)  // disable channel if all config parameters are "not_used"
	{
		currentValue = 0;
		disable();
	}
}
