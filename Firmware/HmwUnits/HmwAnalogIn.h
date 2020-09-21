/*
 * HmwAnalogIn.h
 *
 *  Created on: 26.04.2017
 *      Author: Viktor Pankraz
 *  Changed on: 07.06.2018
 *      Author: loetmeister.de
 */

#ifndef HmwAnalogIn_H
#define HmwAnalogIn_H

#include "HmwChannel.h"
#include <Time/Timestamp.h>
#include <xEeprom.h>


class HmwAnalogIn : public HmwChannel
{
   public:

      enum States
      {
         INIT_ADC,
         START_MEASUREMENT,
         SAMPLE_VALUES,
         SEND_FEEDBACK
      };

      struct Config
      {
         uint8_tx unused;
         uint8_tx minDelta;
         uint16_tx minInterval;	//TODO: keep TX_MINDELAY only?
         uint16_tx maxInterval;
      };

	  static const uint8_t MAX_SAMPLES = 4;

      ////    Constructors and destructors    ////

	  HmwAnalogIn( uint8_t _adcInputPort, uint8_t _adcInputPin, Config* _config );

      ////    Operations    ////

      // definition of needed functions from HBWChannel class
      virtual uint8_t get( uint8_t* data );
      virtual void loop();
	  virtual void checkConfig();

   private:

     static const uint8_t debugLevel;

      ////    Additional operations    ////

   public:
	
	//uint16_t getCurrentValue();
	
	inline void disable()
	{
		nextActionDelay = 0;
	}

   private:


      ////    Attributes    ////

   public:
   
      uint16_t currentValue;

   private:

      ADC_t* adc;

      uint8_t adcInputPin;
	  uint8_t adcInputPort;

      Config* config;

      uint16_t nextActionDelay;

      //uint16_t currentValue;

      uint16_t lastSentValue;

      uint16_t buffer[MAX_SAMPLES] = { 0, 0, 0, 0 };//, 0, 0 };
      uint8_t nextIndex;

};


#endif

