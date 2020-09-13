/*
 * HmwDimmer.h
 *
 * Created: 26.04.2017 09:01:56
 * Author: viktor.pankraz
 */


#ifndef __HMWDIMMER_H__
#define __HMWDIMMER_H__

#include "HmwChannel.h"
#include <PwmOutput.h>
#include <Time/Timestamp.h>
#include <xEeprom.h>


class HmwDimmer : public HmwChannel
{

   public:
      class Config
      {
         struct Options
         {
            uint8_t dimmingMode : 2;
            uint8_t pwmRangeStart : 2;
            uint8_t pwmRangeEnd : 2;
            uint8_t reserved : 1;
            uint8_t logging : 1;
         };

         XEeprom<Options> options;
         uint8_tx reserve;

         public:

            enum Mode
            {
               SWITCH = 0,
               DIMM_PWM,
               DIMM_LEADING,
               DIMM_TRAILING
            };

            enum PwmRangeStart
            {
               _30_PERCENT = 0,
               _20_PERCENT,
               _10_PERCENT,
               _00_PERCENT
            };

            enum PwmRangeEnd
            {
               _70_PERCENT = 0,
               _80_PERCENT,
               _90_PERCENT,
               _100_PERCENT
            };

            Options getOptions() const
            {
               return options.operator*();
            }

            inline bool isLogging() const
            {
               return getOptions().logging;
            }

            inline uint8_t getDimmingMode() const
            {
               return getOptions().dimmingMode;
            }

            inline bool isDimmingModeSwitch() const
            {
               return ( getDimmingMode() == SWITCH );
            }

            inline bool isDimmingModeLeading() const
            {
               return ( getDimmingMode() == DIMM_LEADING );
            }

            inline bool isDimmingModeTrailing() const
            {
               return ( getDimmingMode() == DIMM_TRAILING );
            }

            inline bool isDimmingModePwm() const
            {
               return ( getDimmingMode() == DIMM_PWM );
            }

            inline uint8_t getPwmRangeStart() const
            {
               return getOptions().pwmRangeStart;
            }

            inline uint8_t getPwmRangeEnd() const
            {
               return getOptions().pwmRangeEnd;
            }

            inline void checkOrRestore()
            {
               if ( getOptions().reserved != 1 )
               {
                  Options defaultOptions =
                  {
                     .dimmingMode = DIMM_TRAILING,
                     .pwmRangeStart = _00_PERCENT,
                     .pwmRangeEnd = _100_PERCENT,
                     .reserved = 1,
                     .logging = 1
                  };
                  options = defaultOptions;
               }
               if ( reserve != 0xFF )
               {
                  reserve.update( 0xFF );
               }
            }
      };

      enum Actions
      {
         INACTIVE,
         JUMP_TO_TARGET,
         TOGGLE_TO_COUNTER,
         TOGGLE_INVERS_TO_COUNTER,
         UPDIM,
         DOWNDIM,
         TOGGLEDIM,
         TOGGLEDIM_TO_COUNTER,
         TOGGLEDIM_INVERS_TO_COUNTER,
      };

      enum TimeModes
      {
         TIME_MODE_ABSOLUTE,
         TIME_MODE_MINIMAL,
      };

      struct ActionParameter
      {
         uint8_t actionType    : 4;
         uint8_t multiExecute  : 1;
         uint8_t onDelayMode   : 1;
         uint8_t offTimeMode   : 1;
         uint8_t onTimeMode    : 1;
         uint8_t offLevel;
         uint8_t onMinLevel;
         uint8_t onLevel;
         uint8_t rampStartStep;
         uint8_t offDelayStep;
         uint16_t onDelayTime;
         uint16_t rampOnTime;
         uint16_t onTime;
         uint16_t offDelayTime;
         uint16_t rampOffTime;
         uint16_t offTime;
         uint8_t dimMinLevel;
         uint8_t dimMaxLevel;
         uint8_t dimStep;
         uint8_t jtOnDelay  : 4;
         uint8_t jtRampOn   : 4;
         uint8_t jtOn       : 4;
         uint8_t jtOffDelay : 4;
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
         JT_RAMP_ON = 0x01,
         JT_ON = 0x02,
         JT_OFFDELAY = 0x03,
         JT_RAMP_OFF = 0x04,
         JT_OFF = 0x05,
         JT_NO_JUMP_IGNORE_COMMAND = 0x06,
         // internal states only
         DIM_UP,
         DIM_DOWN,
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


      static const uint8_t MAX_LEVEL = 200;
      static const uint8_t DEFAULT_NORMALIZE_LEVEL = 205;
      static const uint8_t LOOP_PERIOD_MS = 8;
      static const uint16_t MAX_DIMMING_TIME = 300;

   protected:

   private:

      static const uint8_t debugLevel;

      const uint8_t normalizeLevel;

      uint8_t dimmingFactor;

      uint16_t dimmingOffset;

      PwmOutput pwmOutput;

      DigitalOutput enableOutput;

      uint8_t currentLevel;

      Config* config;

      ActionParameter const* actionParameter;

      TimeModes timeMode;

      uint8_t rampStep;

      uint8_t lastKeyNum;

      uint8_t dimTargetLevel;

      uint32_t nextStepTime;


// functions
   public:
      HmwDimmer( PortPin _pwmPin, PortPin _enablePin, Config* _config, uint8_t _normalizeLevel = DEFAULT_NORMALIZE_LEVEL );

      // definition of needed functions from HBWChannel class
      virtual uint8_t get( uint8_t* data );
      virtual void loop();
      virtual void set( uint8_t length, uint8_t const* const data );
      virtual void checkConfig();

      inline const Config& getConfig() const
      {
         return *config;
      }

      inline const PwmOutput& getPwmOutput() const
      {
         return pwmOutput;
      }

      inline const DigitalOutput& getEnableOutput() const
      {
         return enableOutput;
      }

      inline uint8_t getLastKeyNum() const
      {
         return lastKeyNum;
      }

      inline bool isRampRunning() const
      {
         return ( currentState == JT_RAMP_ON ) || ( currentState == JT_RAMP_OFF );
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

   private:

      void calculateRampParameter( uint8_t targetState );

      void calculateDimmingParameter();

      uint8_t getNextDimLevel( bool dimUp );

      void handleStateChart();

      bool handleSpecificState( uint8_t _state );

      void prepareNextState( bool fromAction = false );

}; // HmwDimmer

#endif // __HMWDIMMER_H__
