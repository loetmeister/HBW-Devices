/*
 * MainUnitTests.h
 *
 * Created: 03.12.2016 02:31:44
 *  Author: Viktor Pankraz
 */

#ifndef __MainUnitTests_H__
#define __MainUnitTests_H__

#include <Eeprom.h>
#include <HmwDevice.h>
#include <HmwDimmer.h>
#include <HmwSwitch.h>
#include <HmwLed.h>
#include <HmwKey.h>
#include <HmwBlindActor.h>

#include <catch.hpp>

// this is the EEPROM layout used by one device
struct UnitTestsConfig
{
   HmwDeviceHw::BasicConfig basicConfig; // 0x0000 - 0x0009
   uint8_tx reserved[6];                 // 0x000A - 0x000F
   HmwKey::Config keyCfg;
   HmwSwitch::Config switchCfg;
   HmwDimmer::Config dimCfg;
   HmwBlindActor::Config blCfg;
};

class UnitTestDeviceHw : public HmwDeviceHw
{
// variables
   public:

   protected:

   private:

      UnitTestsConfig* myConfig;
      HmwDimmer hbwDimmer;
      HmwKey hbwKey;
      HmwSwitch hbwSwitch;
      HmwBlindActor hbwBlindActor;

// functions
   public:

      static UnitTestDeviceHw& instance();

      inline HmwDimmer& getHmwDimmer()
      {
         return hbwDimmer;
      }

      inline HmwBlindActor& getHmwBlindActor()
      {
         return hbwBlindActor;
      }      

      inline HmwSwitch& getHmwSwitch()
      {
         return hbwSwitch;
      }

      inline HmwKey& getHmwKey()
      {
         return hbwKey;
      }

      virtual BasicConfig* getBasicConfig()
      {
         return &myConfig->basicConfig;
      }

      virtual inline void enableTranceiver( bool enable )
      {
      }

   private:

      UnitTestDeviceHw();

};

void REQUIRE_STATE_ENTRY_BY_LOOP( HmwChannel& obj, uint32_t delayTime, uint8_t targetState );
void LOOP_FOR_SPECIFIC_TIME( HmwChannel& obj, uint32_t delayTime );

template<typename ActionParameter>
void REQUIRE_STATE_ENTRY_BY_ACTION( HmwChannel& obj, ActionParameter& params, uint8_t targetState, uint32_t delayTime = 0 )
{
   Timestamp startTime;
   while ( startTime.since() < delayTime )
   {
      obj.loop();
   }
   obj.set( sizeof( params ), (uint8_t*)&params );
   REQUIRE( obj.getCurrentState() == targetState );
}

#endif