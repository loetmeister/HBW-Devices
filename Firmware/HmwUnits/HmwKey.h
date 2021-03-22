#ifndef HmwKey_h
#define HmwKey_h

#include "HmwChannel.h"
#include <DigitalInput.h>
#include <Time/Timestamp.h>
#include <xEeprom.h>


// Class HBWKey
class HmwKey : public HmwChannel
{
   public:

      static const uint8_t DEBOUNCE_TIME = 100;
	  static const uint16_t DEBOUNCE_TIME_MOTION_SENSOR = 330;

      class Config
      {
         enum OptionMask
         {
            INPUTTYPE_MASK = 0x07,
            UNLOCKED_MASK = 0x08,
            PULLUP_MASK = 0x10,
            NOT_INVERT_MASK = 0x20,
            REPEAT_ON_LONG_PRESS_MASK = 0x40,
            FEEDBACK_MASK = 0x80,
         };

         uint8_tx options;
         uint8_tx long_press_time;

         public:

            enum InputType
            {
               SWITCH = 0,
               PUSHBUTTON,
               MOTIONSENSOR,
               MAX_INPUTTYPE
            };

            enum Consts
            {
               DEFAULT_LONG_PRESS_TIME = 4
            };

            inline uint8_t getInputType() const
            {
               return ( options & INPUTTYPE_MASK );
            }

            inline void setInputType( uint8_t type )
            {
               if ( type < MAX_INPUTTYPE )
               {
                  options.update( ( options & ~INPUTTYPE_MASK ) | type );
               }
            }

            inline bool isPushButton() const
            {
               return getInputType() == PUSHBUTTON;
            }

            inline bool isSwitch() const
            {
               return getInputType() == SWITCH;
            }

            inline bool isMotionSensor() const
            {
               return getInputType() == MOTIONSENSOR;
            }

            inline bool isUnlocked() const
            {
               return options & UNLOCKED_MASK;
            }

            inline bool isPullUp() const
            {
               return options & PULLUP_MASK;
            }

            inline bool isInverted() const
            {
               return !( options & NOT_INVERT_MASK );
            }

            inline bool isFeedbackEnabled() const
            {
               return options & FEEDBACK_MASK;
            }

            inline bool repeatOnLongPress() const
            {
               return options & REPEAT_ON_LONG_PRESS_MASK;
            }

            inline uint8_t getLongPressTime() const
            {
               return long_press_time;
            }

            inline void setInverted( bool inverted )
            {
               options.update( ( options & ~NOT_INVERT_MASK ) | ( inverted ? 0 : NOT_INVERT_MASK ) );
            }

            inline void setLongPressTime( uint8_t time )
            {
               long_press_time.update( time );
            }
      };

      HmwKey( PortPin _pin, Config* _config, HmwChannel* _feedbackChannel = NULL );

      inline void setFeedbackChannel( HmwChannel* _feedbackChannel )
      {
         feedbackChannel = _feedbackChannel;
      }

      inline void setUnlocked( bool _unlocked )
      {
         unlocked = _unlocked;
         if ( feedbackChannel && !unlocked )
         {
            uint8_t data = KEY_FEEDBACK_OFF;
            feedbackChannel->set( sizeof( data ), &data );
         }
      }

      inline void setPulldownSupport( bool enable )
      {
         pulldownSupported = enable;
      }

      inline bool isUnlocked() const
      {
         return config->isUnlocked() && unlocked;
      }

      inline bool isPressed() const
      {
         return !digitalIn.isSet();
      }

      inline const Config& getConfig() const
      {
         return *config;
      }

      inline const DigitalInput& getDigitalInput() const
      {
         return digitalIn;
      }
      virtual uint8_t get( uint8_t* data );

      virtual void loop();

      virtual void checkConfig();

      void setFeedbackChannel( uint8_t cmd )
	  {
		  if ( feedbackChannel && config->isFeedbackEnabled() )
		  {
			  feedbackChannel->set( 1, &cmd );
		  }
	  }

   protected:

      void resetChannel();

      void handlePushButtonSignal();

      void handleSwitchSignal();

      void handleMotionSensorSignal();

   private:

      bool unlocked;

      bool pulldownSupported;
	  
	  bool isStartUp;

      uint8_t keyPressNum;

      Config* config;

      HmwChannel* feedbackChannel;

      DigitalInput digitalIn;

      Timestamp keyPressedTimestamp;   // Zeit, zu der die Taste gedrueckt wurde (fuer's Entprellen)

      Timestamp lastSentLong;          // Zeit, zu der das letzte Mal longPress gesendet wurde
	  
   private:
	  static const SystemTime::time_t MOTION_SENSOR_STARTUP_BLOCKING_TIME = 58000;	// consider motion sensor continuously active since startup as really active after this time (58 seconds)
};

#endif