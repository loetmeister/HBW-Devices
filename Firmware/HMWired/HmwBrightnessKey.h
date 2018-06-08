/*
 * HmwBrightness.h
 *
 * This is a 'virtual' channel, triggered by HmwBrightnessSwitch.
 * The current brightness read from HmwBrightnessSwitch defines if
 * a key event is send on the bus or not.
 * It acts as a key for peering role. The threshold is configured in
 * the channel settings.
 *
 * This is used to peer a real key (input) with an external actor, allowing a level (brightness) as condition.
 *
 *  Created on: 07.06.2018
 *      Author: loetmeister.de
 */

#ifndef HmwBrightnessKey_H
#define HmwBrightnessKey_H

#include "HmwChannel.h"
#include <Time/Timestamp.h>
#include <xEeprom.h>

#define DEFAULT_TRIGGER_LEVEL 20

class HmwBrightnessSwitch;	// forward declare this class

class HmwBrightnessKey : public HmwChannel
{
   public:

      struct Config
      {
         uint8_tx options;
         uint8_tx triggerLevel;
      };
	  
      enum OptionMask
      {
	      CON_LE_GE_MASK = 0x80
      };

      inline bool compareConditionLE() const
      {
	      return config->options & CON_LE_GE_MASK;
      }

      ////    Constructors and destructors    ////

      HmwBrightnessKey( HmwBrightnessSwitch& _linkedBrightnessSwitchChannel, Config* _config );

      ////    Operations    ////

      // definition of needed functions from HBWChannel class
      virtual void loop( uint8_t channel );
	  virtual void checkConfig();

   private:


      ////    Additional operations    ////

   public:


   private:


      ////    Attributes    ////

   public:
   
   protected:

      void resetChannel();

   private:

      Config* config;

      HmwBrightnessSwitch* linkedBrightnessSwitchChannel;
	  
	  uint8_t keyPressNum;

};


#endif

