/*
 * MainUnitTests.cpp
 *
 * Created: 03.12.2016 02:31:44
 *  Author: Viktor Pankraz
 */

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "MainUnitTests.h"


void debug( char c )
{
   std::cout << c;
}

UnitTestDeviceHw& UnitTestDeviceHw::instance()
{
   static UnitTestDeviceHw myHardware;
   return myHardware;
}

UnitTestDeviceHw::UnitTestDeviceHw() :
   myConfig( (UnitTestsConfig*)Eeprom::setup( 1024 ) ),
   hbwDimmer( PortPin( Port0, Pin0 ), PortPin( Port0, Pin1 ), &myConfig->dimCfg ),
   hbwKey( PortPin( Port0, Pin2 ), &myConfig->keyCfg ),
   hbwSwitch( PortPin( Port0, Pin3 ), &myConfig->switchCfg ),
   hbwBlindActor( PortPin( Port0, Pin4 ), PortPin( Port0, Pin5 ), &myConfig->blCfg )
{
   Eeprom::erase();
   HmwDevice::setHardware( 0, this );
   Logger::instance().setStream( debug );
}

void LOOP_FOR_SPECIFIC_TIME( HmwChannel& obj, uint32_t delayTime )
{
   Timestamp startTime;
   while ( startTime.since() < delayTime )
   {
      obj.loop();
   }
}

void REQUIRE_STATE_ENTRY_BY_LOOP( HmwChannel& obj, uint32_t delayTime, uint8_t targetState )
{
   const uint32_t maxDeviation = 100;
   Timestamp startTime;
   Timestamp stopTime = startTime.get() + delayTime;
   while ( obj.getCurrentState() != targetState )
   {
      Timestamp loopStartTime;
      obj.loop();
      if ( loopStartTime.since() > maxDeviation )
      {
         CAPTURE( loopStartTime.get() );
         CHECK( loopStartTime.since() <= maxDeviation );
      }
      if ( stopTime.since() > maxDeviation )
      {
         break;
      }
   }
   SystemTime::time_t now = SystemTime::now();
   CAPTURE( startTime.get() );
   CAPTURE( stopTime.get() );
   CAPTURE( now );

   REQUIRE( obj.getCurrentState() == targetState );
   CHECK( ( now + maxDeviation ) >= stopTime.get() );
   CHECK( stopTime.since() < maxDeviation );
}