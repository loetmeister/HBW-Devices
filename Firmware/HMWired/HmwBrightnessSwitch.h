/*
 * HmwBrightness.h
 *
 * This is a 'virtual' channel, reading an analog 12bit value from the
 * linked HmwAnalogIn channel and creating an moving average.
 * The average is stored as 0...200 value, converted to 0...100% with a
 * factor of 2 by FHEM or CCU.
 * It acts as a switch for peering role, but appears as brightness level channel.
 *
 * This is used to peer a real key (input) with an external actor, allowing a level (brightness) as condition.
 *
 *  Created on: 07.06.2018
 *      Author: loetmeister.de
 */

#ifndef HmwBrightnessSwitch_H
#define HmwBrightnessSwitch_H

#include "HmwChannel.h"
#include <Time/Timestamp.h>
#include <xEeprom.h>

#define MAX_SAMPLES 8

class HmwAnalogIn;	// forward declare this class

class HmwBrightnessSwitch : public HmwChannel
{
   public:

      struct Config
      {
         uint8_tx unused;
         uint8_tx interval;
      };


      ////    Constructors and destructors    ////

      HmwBrightnessSwitch( HmwAnalogIn& _linkedAnalogChannel, Config* _config );

      ////    Operations    ////

      // definition of needed functions from HBWChannel class
      virtual uint8_t get( uint8_t* data );
	  virtual void set( uint8_t length, uint8_t const* const data );
      virtual void loop( uint8_t channel );

   private:


      ////    Additional operations    ////

   public:


   private:


      ////    Attributes    ////

   public:

      bool triggered;
	  
	  uint8_t currentValue;

   private:

      Config* config;

      HmwAnalogIn* linkedAnalogChannel;

      uint16_t nextActionDelay;

      Timestamp lastActionTime;
	  
	  uint8_t buffer[MAX_SAMPLES] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	  int16_t result_sum;
	  uint8_t index;
	  uint8_t count;

};


#endif

