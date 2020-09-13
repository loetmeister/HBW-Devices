/*
 * HmwBlindActor.h
 *
 * Created: 26.04.2019 09:01:56
 * Author: viktor.pankraz
 */


#ifndef __HMWBLINDACTOR_H__
#define __HMWBLINDACTOR_H__

#include "HmwChannel.h"
#include <DigitalOutput.h>
#include <Time/Timestamp.h>
#include <xEeprom.h>


class HmwBlindActor : public HmwChannel
{

   public:
      class Config
      {
         struct Options
         {
            uint8_t reserved : 6;
            uint8_t keepLogicalEnd : 1;
            uint8_t logging : 1;
         };

         XEeprom<Options> options;

         public:

            static const uint8_t DEFAULT_CHANGE_OVER_DELAY = 5;
            static const uint8_t DEFAULT_RUN_COUNTER = 0;
            static const uint16_t DEFAULT_RUN_TIME = 500;

            uint8_tx changeOverDelay;
            uint8_tx runCounter;
            uint16_tx runTimeBottomTop;
            uint16_tx runTimeTopBottom;
            uint8_tx lastPosition;

            Options getOptions() const
            {
               return options.operator*();
            }

            inline bool isLogging() const
            {
               return getOptions().logging;
            }

            inline bool isKeepingLogicalEnd() const
            {
               return getOptions().keepLogicalEnd;
            }

            inline uint16_t getRunTimeStepUp() const
            {
               return runTimeBottomTop / 2; // SystemTime::S / 10 / MAX_LEVEL is 1.95
            }

            inline uint16_t getChangeOverDelayTime() const
            {
               return changeOverDelay;
            }

            inline uint16_t getRunTimeStepDown() const
            {
               return runTimeTopBottom / 2; // SystemTime::S / 10 / MAX_LEVEL is 1.95
            }

            inline void checkOrRestore()
            {
               if ( getOptions().reserved != 0x3F )
               {
                  Options defaultOptions =
                  {
                     .reserved = 0x3F,
                     .keepLogicalEnd = 1,
                     .logging = 1
                  };
                  options = defaultOptions;
               }
               if ( ( changeOverDelay < 5 ) || ( changeOverDelay > 250 ) )
               {
                  changeOverDelay.update( DEFAULT_CHANGE_OVER_DELAY );
               }
               if ( runCounter > 100 )
               {
                  runCounter.update( DEFAULT_RUN_COUNTER );
               }
               if ( ( runTimeBottomTop < 1 ) || ( runTimeBottomTop > 60000 ) )
               {
                  runTimeBottomTop.update( DEFAULT_RUN_TIME );
               }
               if ( ( runTimeTopBottom < 1 ) || ( runTimeTopBottom > 60000 ) )
               {
                  runTimeTopBottom.update( DEFAULT_RUN_TIME );
               }
               if ( lastPosition > HmwChannel::MAX_LEVEL )
               {
                  lastPosition.update( 0 );
               }
            }
      };

      enum Actions
      {
         INACTIVE,
         ACTIVE,
      };

      enum DrivingModes
      {
         DRIVE_VIA_NEXT_END,
         DRIVE_VIA_UPPER_END,
         DRIVE_VIA_LOWER_END,
         DRIVE_VIA_DIRECTLY,
      };

      enum TimeModes
      {
         TIME_MODE_MINIMAL,
         TIME_MODE_ABSOLUTE,
      };

      struct ActionParameter
      {
         uint8_t actionType    : 1;
         uint8_t notUsed       : 2;
         uint8_t toggleUse     : 1;
         uint8_t drivingMode   : 2;
         uint8_t offTimeMode   : 1;
         uint8_t onTimeMode    : 1;
         uint8_t offLevel;
         uint8_t onLevel;
         uint16_t onDelayTime;
         uint16_t offDelayTime;
         uint16_t onTime;
         uint16_t offTime;
         uint8_t maxTimeFirstDirection;
         uint8_t jtOnDelay  : 4;
         uint8_t jtRefOn    : 4;
         uint8_t jtRampOn   : 4;
         uint8_t jtOn       : 4;
         uint8_t jtOffDelay : 4;
         uint8_t jtRefOff   : 4;
         uint8_t jtRampOff  : 4;
         uint8_t jtOff      : 4;

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
         JT_REF_ON,
         JT_RAMP_ON,
         JT_ON,
         JT_OFFDELAY,
         JT_REF_OFF,
         JT_RAMP_OFF,
         JT_OFF,
         JT_NO_JUMP_IGNORE_COMMAND,
      };

      union StateFlags
      {
         struct
         {
            uint8_t notUsed : 4; // lowest 4 bit are not used, based on XML state_flag definition
            uint8_t upDown  : 2; // dim up = 1 or down = 2
            uint8_t working : 1; // true, if working
            uint8_t   : 1;
         } flags;

         uint8_t byte;
      };

   protected:

   private:

      static const uint8_t debugLevel;

      DigitalOutput moveUpOutput;

      DigitalOutput moveDownOutput;

      uint8_t currentLevel;

      Config* config;

      ActionParameter const* actionParameter;

      TimeModes timeMode;

      Timestamp lastRepeatedKeyPressTime;

      uint8_t lastKeyNum;

      uint8_t targetLevel;

// functions
   public:
      HmwBlindActor( PortPin _moveUp, PortPin _moveDown, Config* _config );

      // definition of needed functions from HBWChannel class
      virtual uint8_t get( uint8_t* data );
      virtual void loop();
      virtual void set( uint8_t length, uint8_t const* const data );
      virtual void checkConfig();

      inline const Config& getConfig() const
      {
         return *config;
      }

      inline const DigitalOutput& getMoveUpOutput() const
      {
         return moveUpOutput;
      }

      inline const DigitalOutput& getMoveDownOutput() const
      {
         return moveDownOutput;
      }

      inline uint8_t getLastKeyNum() const
      {
         return lastKeyNum;
      }

      inline bool isInRampOn() const
      {
         return ( currentState == JT_RAMP_ON ) || ( currentState == JT_REF_ON );
      }

      inline bool isInRampOff() const
      {
         return ( currentState == JT_RAMP_OFF ) || ( currentState == JT_REF_OFF );
      }

      inline bool isRampRunning() const
      {
         return isInRampOff() || isInRampOn();
      }

      inline bool isNextRampOn() const
      {
         return ( nextState == JT_RAMP_ON ) || ( nextState == JT_REF_ON );
      }

      inline bool isNextRampOff() const
      {
         return ( nextState == JT_RAMP_OFF ) || ( nextState == JT_REF_OFF );
      }

      inline bool isNextRampRunning() const
      {
         return isNextRampOff() || isNextRampOn();
      }

      inline bool isAbsoluteTimeMode() const
      {
         return timeMode == TIME_MODE_ABSOLUTE;
      }

      inline bool isMinimalTimeMode() const
      {
         return timeMode == TIME_MODE_MINIMAL;
      }

   protected:

      void setLevel( uint8_t level, bool withLogging = true );

      void setTargetLevel( uint8_t level );

      void stop();

   private:

      void handleStateChart();

      bool handleSpecificState( uint8_t _state );

      void prepareNextState( bool fromAction = false );

}; // HmwBlindActor

#endif // __HMWBLINDACTOR_H__
