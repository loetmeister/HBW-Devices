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

#include "HmwLed.h"
#include <Time/Timestamp.h>
#include <xEeprom.h>


class HmwAnalogIn;	// forward declare this class

class HmwBrightnessSwitch : public HmwChannel
{
   public:

      struct Config
      {
         uint8_tx unused;
         uint8_tx interval;
      };

      static const uint8_t MAX_SAMPLES = 8;

	// using peering layout of HmwLed!	//TOD: add as struct? HBWLinkLed works differently to dimmer...
      //struct ActionParameter
      //{
	      ////uint32_t sensorAddress;
	      ////uint8_t sensorChannel;
	      ////uint8_t ownChannel;
	      ////uint8_t shortActionType : 3;
	      ////uint8_t longActionType : 3;
		  //uint8_t onOffLevelOrCmd;
	      //uint8_t offLevel;
	      //uint8_t onLevel;
	      ////uint8_t longOnLevel;
	      ////uint8_t longOffLevel;
	      //uint8_t blinkOnTime;
	      //uint8_t blinkOffTime;
	      //uint8_t blockingTime;
	      ////uint8_t reserved[2];
      //};
	  //
      //struct LinkCommand
      //{
	      ////uint8_t keyNum;
	      //ActionParameter* actionParameter;
      //};
	  
      ////    Constructors and destructors    ////

      HmwBrightnessSwitch( HmwAnalogIn& _linkedAnalogChannel, Config* _config );

      ////    Operations    ////

      // definition of needed functions from HBWChannel class
      virtual uint8_t get( uint8_t* data );
	  virtual void set( uint8_t length, uint8_t const* const data );
      virtual void loop();
	  virtual void checkConfig();

   private:


      ////    Additional operations    ////

   public:


   private:


      ////    Attributes    ////

   public:

	  uint8_t triggered;
	  
	  uint8_t currentValue;

   private:

      Config* config;

      HmwAnalogIn* linkedAnalogChannel;

      uint16_t nextActionDelay;

      Timestamp lastActionTime;
	  
	  //ActionParameter const* actionParameter;
	  
	  uint8_t buffer[MAX_SAMPLES] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	  int16_t result_sum;
	  uint8_t index;
	  uint8_t count;
	  
	  union tag_state_flags {
		struct state_flags {
			uint8_t notUsed :4; // lowest 6 bit are not used, based on XML state_flag definition (index="12.4" size="0.3")
			uint8_t active  :1; // condition is currently met
			uint8_t le  :1; // less or equals (false: ge, greater or equals)
			uint8_t blockingTimeActive  :1; 
		} element;
		uint8_t byte:8;
	  } stateFlags;
};


#endif

