/*
 * HmwSwitch.h
 *
 * Created: 26.04.2017 09:01:56
 * Author: viktor.pankraz
 */


#ifndef __HMWSWITCH_H__
#define __HMWSWITCH_H__

#include "HmwChannel.h"
#include <DigitalOutput.h>
#include <Time/Timestamp.h>
#include <xEeprom.h>


class HmwSwitch : public HmwChannel
{

   public:
      class Config
      {
         struct Options
         {
            uint8_t logging : 1;
            uint8_t reserved : 7;
         };

         XEeprom<Options> options;
         uint8_tx reserve;

         public:

            Options getOptions() const
            {
               return options.operator*();
            }

            inline bool isLogging() const
            {
               return getOptions().logging;
            }

            inline void checkOrRestore()
            {
               if ( getOptions().reserved != 0x7F )
               {
                  Options defaultOptions =
                  {
                     .logging = 1,
                     .reserved = 0x7F
                  };
                  options = defaultOptions;
               }
               if ( reserve != 0xFF )
               {
                  reserve.update( 0xFF );
               }
            }
      } __attribute__( ( packed ) );

      enum Actions
      {
         INACTIVE,
         ACTIVE,
      };

      enum ToggleModes
      {
         DONT_USE,
         DIRECT,
         INVERTED,
      };

      enum TimeModes
      {
         TIME_MODE_ABSOLUTE,
         TIME_MODE_MINIMAL,
      };

      struct ActionParameter
      {
         uint8_t actionType    : 1;
         uint8_t notUsed       : 3;
         uint8_t toggleUse     : 2;
         uint8_t offTimeMode   : 1;
         uint8_t onTimeMode    : 1;
         uint16_t onDelayTime;
         uint16_t onTime;
         uint16_t offDelayTime;
         uint16_t offTime;
         uint16_t jtOnDelay    : 3;
         uint16_t jtOn         : 3;
         uint16_t jtOffDelay   : 3;
         uint16_t jtOff        : 3;

         inline bool isOffTimeAbsolute() const
         {
            return offTimeMode == TIME_MODE_ABSOLUTE;
         }
         inline bool isOnTimeAbsolute() const
         {
            return onTimeMode == TIME_MODE_ABSOLUTE;
         }
         inline bool isOffTimeMinimal() const
         {
            return offTimeMode == TIME_MODE_MINIMAL;
         }
         inline bool isOnTimeMinimal() const
         {
            return onTimeMode == TIME_MODE_MINIMAL;
         }
      };

      struct LinkCommand
      {
         uint8_t keyNum;
         ActionParameter* actionParameter;
      };

      enum States
      {
         JT_ONDELAY = 0x00,
         JT_ON = 0x01,
         JT_OFFDELAY = 0x02,
         JT_OFF = 0x03,
         JT_NO_JUMP_IGNORE_COMMAND = 0x04,
      };

      union StateFlags
      {
         struct
         {
            uint8_t notUsed : 4; // lowest 4 bit are not used, based on XML state_flag definition
            uint8_t state   : 2; // on = 1 or off = 2
            uint8_t working : 1; // true, if working
            uint8_t   : 1;
         } flags;

         uint8_t byte;
      };

   protected:

   private:

      static const uint8_t debugLevel;

      DigitalOutput output;

      uint8_t currentLevel;

      uint8_t lastReportedLevel;

      Config* config;

      ActionParameter const* actionParameter;

      uint8_t lastKeyNum;

      TimeModes timeMode;


// functions
   public:
      HmwSwitch( PortPin _pwmPin, Config* _config );

      // definition of needed functions from HBWChannel class
      virtual uint8_t get( uint8_t* data );
      virtual void set( uint8_t length, uint8_t const* const data );
      virtual void loop();
      virtual void checkConfig();

      inline bool isAbsoluteTimeMode() const
      {
         return timeMode == TIME_MODE_ABSOLUTE;
      }

      inline bool isMinimalTimeMode() const
      {
         return timeMode == TIME_MODE_MINIMAL;
      }

      inline const Config& getConfig() const
      {
         return *config;
      }

      inline const DigitalOutput& getOutput() const
      {
         return output;
      }

      inline uint8_t getLastKeyNum() const
      {
         return lastKeyNum;
      }

   protected:

      void setLevel( uint8_t level, bool withLogging = true );

   private:

      void prepareNextState( bool fromAction = false );

      void handleStateChart();


}; // HmwSwitch

#endif // __HMWSWITCH_H__
