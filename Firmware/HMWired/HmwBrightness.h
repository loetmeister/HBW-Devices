/*
 * HmwBrightness.h
 *
 *  Created on: 07.05.2018
 *      Author: loetmeister.de
 */

#ifndef HmwBrightness_H
#define HmwBrightness_H

#include "HmwChannel.h"
#include <Time/Timestamp.h>
#include <xEeprom.h>

#define MAX_SAMPLES 8

class HmwAnalogIn;	// forward declare this class

class HmwBrightness : public HmwChannel
{
   public:

/*      enum State
      {
         SAMPLE_VALUES,
         SEND_FEEDBACK
      };*/

      struct Config
      {
         uint8_tx unused;
         uint8_tx samples;
         uint8_tx interval;
      };


      ////    Constructors and destructors    ////

      HmwBrightness( HmwAnalogIn& _linkedAnalogChannel, Config* _config );

      ////    Operations    ////

      // definition of needed functions from HBWChannel class
      virtual uint8_t get( uint8_t* data );
      virtual void loop( uint8_t channel );

   private:


      ////    Additional operations    ////

   public:


   private:


      ////    Attributes    ////

   public:

   private:

      Config* config;

      HmwAnalogIn* linkedAnalogChannel;

      uint16_t nextActionDelay;

      uint8_t currentValue;

      Timestamp lastActionTime;
	  
	  uint8_t buffer[MAX_SAMPLES] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	  int16_t result_sum;
	  uint8_t index;
	  uint8_t count;

};


#endif

