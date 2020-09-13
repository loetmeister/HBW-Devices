/*
 * HmwChannel.h
 *
 * Created: 18.11.2017 23:25:12
 * Author: viktor.pankraz
 */


#ifndef __HMWCHANNEL_H__
#define __HMWCHANNEL_H__

#ifndef MAX_CHANNELS
#define MAX_CHANNELS 38
#endif


#include <Time/Timestamp.h>

class HmwChannel
{
// variables
   public:

      enum BlindActorCommands
      {
         STOP = 201,
      };

      enum GenericChannelCommands
      {
         KEY_FEEDBACK_ON = 252,
         KEY_FEEDBACK_OFF = 253,
         LINK_ACTION = 254,
         BROADCAST_LINK_ACTION = 255,
      };

      enum Type
      {
         UNKNOWN,
         HMW_KEY,
         HMW_LED,
         HMW_DIMMER,
         HMW_DS18X20,
         HMW_SHTC3,
         HMW_SWITCH,
         HMW_BLIND_ACTOR,
		 HMW_SHT3x
      };

      static const uint16_t ENDLESS_TIME = 0xC000;

      static const uint16_t REPEAT_LONG_PRESS_TIME = 300;

      static const uint16_t REPEAT_LONG_PRESS_TIMEOUT = 450;

      static const uint8_t MAX_LEVEL = 200;

   protected:

      Type type;
      uint8_t channelId;
      uint8_t currentState;
      uint8_t nextState;
      Timestamp nextActionTime;
      Timestamp nextFeedbackTime;

   private:
      static uint8_t numChannels;
      static HmwChannel* instances[MAX_CHANNELS];

// functions
   public:

      static inline uint8_t getNumChannels()
      {
         return numChannels;
      }

      static inline HmwChannel* getChannel( uint8_t channel )
      {
         return instances[channel];
      }

      inline bool isOfType( Type _type ) const
      {
         return type == _type;
      }

      inline uint8_t getCurrentState() const
      {
         return currentState;
      }

      inline uint8_t getNextState() const
      {
         return nextState;
      }

      inline void disable()
      {
         nextActionTime.reset();
      }

      inline void enable( uint16_t _delay = 0 )
      {
         nextActionTime = SystemTime::now() + _delay;
      }

      bool inline isValidActionTime( uint16_t time ) const
      {
         return ( time < ENDLESS_TIME );
      }

      inline const Timestamp& getNextActionTime() const
      {
         return nextActionTime;
      }

      inline const Timestamp& getNextFeedbackTime() const
      {
         return nextFeedbackTime;
      }

      inline bool isNextActionPending() const
      {
         return nextActionTime.isValid() && nextActionTime.since();
      }

      inline bool isWorkingState() const
      {
         return nextActionTime.isValid();
      }

      uint32_t convertToTime( uint16_t value ) const;

      virtual void set( uint8_t length, uint8_t const* const data );
      virtual uint8_t get( uint8_t* data );  // returns length, data must be big enough
      virtual void loop();
      virtual void checkConfig();

   protected:

      HmwChannel();
      void checkLogging( bool enabled );
      bool handleFeedback( uint32_t nextFeedbackDelay = 0 );

      inline void setMainState( uint8_t _state )
      {
         currentState = _state;
      }

   private:


}; // HmwChannel

#endif // __HMWCHANNEL_H__
