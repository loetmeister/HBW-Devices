/*
 * HmwLed.h
 *
 * Created: 26.04.2017 09:01:56
 * Author: viktor.pankraz
 */


#ifndef __HMWLED_H__
#define __HMWLED_H__

#include "HmwChannel.h"
#include <PwmOutput.h>
#include <xEeprom.h>


class HmwLed : public HmwChannel
{

   public:
      class Config
      {
         enum OptionMask
         {
            LOGGING_MASK = 0x80,
            PWM_RANGE_MASK = 0x7F
         };

         uint8_tx options;

         uint8_tx reserve;

         public:
            inline bool isLogging() const
            {
               return options & LOGGING_MASK;
            }

            inline uint8_t getPwmRange()
            {
               return ( options & PWM_RANGE_MASK );
            }

            inline void setPwmRange( uint8_t value )
            {
               options.update( ( options & ~PWM_RANGE_MASK ) | ( value & PWM_RANGE_MASK ) );
            }
      };

      enum States
      {
         ON = 201,
         OFF,
         TOGGLE,
         BLINK_ON,
         BLINK_TOGGLE,
      };

      union StateFlags
      {
         struct
         {
            uint8_t notUsed : 4; // lowest 4 bit are not used, based on XML state_flag definition
            uint8_t state   : 3; // enum States mapped to 0-5
            uint8_t working : 1; // true, if blinking
         } flags;

         uint8_t byte;
      };

      static const uint8_t MAX_LEVEL = 200;
      static const uint8_t NORMALIZE_LEVEL = 8;
      static const uint16_t MAX_LEVEL_PERIOD = MAX_LEVEL * 100 / NORMALIZE_LEVEL;

   protected:
   private:
      static const uint8_t debugLevel;
      Config* config;
      PwmOutput pwmOutput;
      bool feedbackCmdActive;
      uint8_t currentLevel;
      uint8_t onLevel;
      uint8_t offLevel;
      uint8_t blinkOnTime;
      uint8_t blinkOffTime;
      uint8_t blinkQuantity;
	  uint8_t lastKeyNum;

      const uint8_t defaultPwmRange;

// functions
   public:
      HmwLed( PortPin _portPin, Config* _config, bool _inverted = false, uint8_t _defaultPwmRange = 100 );

      // definition of needed functions from HBWChannel class
      virtual uint8_t get( uint8_t* data );
      virtual void set( uint8_t length, uint8_t const* const data );
      virtual void loop();
      virtual void checkConfig();

   protected:

      void setLevel( uint8_t );

      uint8_t getLevel() const;

   private:
      inline bool isToggleCmd( uint8_t cmd )
      {
         return cmd == TOGGLE;
      }

      inline bool isBlinkOnCmd( uint8_t cmd )
      {
         return cmd == BLINK_ON;
      }

      inline bool isBlinkToggleCmd( uint8_t cmd )
      {
         return cmd == BLINK_TOGGLE;
      }

      inline bool isKeyFeedbackOnCmd( uint8_t cmd )
      {
         return cmd == KEY_FEEDBACK_ON;
      }

      inline bool isKeyFeedbackOffCmd( uint8_t cmd )
      {
         return cmd == KEY_FEEDBACK_OFF;
      }

      inline bool isLogicalOn( void )
      {
         return currentState != OFF;
      }

}; // HmwLed

#endif // __HMWLED_H__
