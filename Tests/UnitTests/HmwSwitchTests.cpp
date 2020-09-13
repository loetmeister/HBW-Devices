/*
 * HmwSwitchTests.cpp
 *
 * Created: 03.12.2016 02:31:44
 *  Author: Viktor Pankraz
 */

#include "MainUnitTests.h"

TEST_CASE( "Check HmwSwitch start up conditions", "[HmwSwitch]" )
{
   HmwSwitch& mySwitch = UnitTestDeviceHw::instance().getHmwSwitch();
   uint8_t testData[128];

   for ( uint8_t i = 0; i < 2; i++ )
   {
      CHECK( mySwitch.getCurrentState() == HmwSwitch::JT_OFF );
      CHECK( mySwitch.get( testData ) == 1 ); // size of data should be 1 Byte
      CHECK( testData[0] == 0 ); // currentLevel has to be 0
      CHECK( mySwitch.getNextActionTime().isValid() == false ); // no next action is expected after startup
      CHECK( mySwitch.getNextFeedbackTime().isValid() == false );  // no feedback expected after startup
      mySwitch.loop();
      SystemTime::waitMs( 1 );
   }
   CHECK( mySwitch.getOutput().isSet() == false );  // outputPin should not be set in state OFF
}

TEST_CASE( "Check HmwSwitch restoring config to default", "[HmwSwitch]" )
{
   HmwSwitch& mySwitch = UnitTestDeviceHw::instance().getHmwSwitch();
   mySwitch.checkConfig();

   const HmwSwitch::Config& config = mySwitch.getConfig();
   CHECK( config.getOptions().logging == 1 );
   CHECK( config.getOptions().reserved == 0x7F );
   CHECK( config.isLogging() == true );
}


TEST_CASE( "Check HmwSwitch profile On/Off behavior", "[HmwSwitch]" )
{
   HmwSwitch& mySwitch = UnitTestDeviceHw::instance().getHmwSwitch();
   mySwitch.checkConfig();

   HmwSwitch::ActionParameter actionParams[] =
   {
      // no on delay, endless on, no off delay, endless off
      {
         .actionType = 1,
         .notUsed = 0,
         .toggleUse = 0,
         .offTimeMode = 1,
         .onTimeMode = 1,
         .onDelayTime = 0,
         .onTime = 0xC000,
         .offDelayTime = 0,
         .offTime = 0xC000,
         .jtOnDelay = HmwSwitch::JT_ON,
         .jtOn = HmwSwitch::JT_OFFDELAY,
         .jtOffDelay = HmwSwitch::JT_OFF,
         .jtOff = HmwSwitch::JT_ONDELAY,
      },
      // 200ms on delay, endless on, no off delay, endless off
      {
         .actionType = 1,
         .notUsed = 0,
         .toggleUse = 0,
         .offTimeMode = 1,
         .onTimeMode = 1,
         .onDelayTime = 2,
         .onTime = 0xC000,
         .offDelayTime = 0,
         .offTime = 0xC000,
         .jtOnDelay = HmwSwitch::JT_ON,
         .jtOn = HmwSwitch::JT_OFFDELAY,
         .jtOffDelay = HmwSwitch::JT_OFF,
         .jtOff = HmwSwitch::JT_ONDELAY,
      },
      // 200ms on delay, 500ms on, no off delay, endless off
      {
         .actionType = 1,
         .notUsed = 0,
         .toggleUse = 0,
         .offTimeMode = 1,
         .onTimeMode = 1,
         .onDelayTime = 2,
         .onTime = 5,
         .offDelayTime = 0,
         .offTime = 0xC000,
         .jtOnDelay = HmwSwitch::JT_ON,
         .jtOn = HmwSwitch::JT_OFFDELAY,
         .jtOffDelay = HmwSwitch::JT_OFF,
         .jtOff = HmwSwitch::JT_ONDELAY,
      },
      // 200ms on delay, 500ms on, 100ms off delay, endless off
      {
         .actionType = 1,
         .notUsed = 0,
         .toggleUse = 0,
         .offTimeMode = 1,
         .onTimeMode = 1,
         .onDelayTime = 2,
         .onTime = 5,
         .offDelayTime = 1,
         .offTime = 0xC000,
         .jtOnDelay = HmwSwitch::JT_ON,
         .jtOn = HmwSwitch::JT_OFFDELAY,
         .jtOffDelay = HmwSwitch::JT_OFF,
         .jtOff = HmwSwitch::JT_ONDELAY,
      },
      // 200ms on delay, 500ms on, 100ms off delay, 200ms off
      {
         .actionType = 1,
         .notUsed = 0,
         .toggleUse = 0,
         .offTimeMode = 1,
         .onTimeMode = 1,
         .onDelayTime = 2,
         .onTime = 5,
         .offDelayTime = 1,
         .offTime = 2,
         .jtOnDelay = HmwSwitch::JT_ON,
         .jtOn = HmwSwitch::JT_OFFDELAY,
         .jtOffDelay = HmwSwitch::JT_OFF,
         .jtOff = HmwSwitch::JT_ONDELAY,
      },
   };

   const int parameterSize = sizeof( actionParams ) / sizeof( HmwSwitch::ActionParameter );

   // select the parameter set
   auto idx = GENERATE( range( 0, parameterSize ) );
   CAPTURE( idx );

   // prepare object for test
   uint8_t level = 0;
   mySwitch.set( sizeof( level ), &level );
   mySwitch.loop();
   REQUIRE( mySwitch.getCurrentState() == HmwSwitch::JT_OFF );

   // test JT_OFF -> JT_ONDELAY
   HmwSwitch::LinkCommand cmd;
   cmd.keyNum = mySwitch.getLastKeyNum() + 1;
   cmd.actionParameter = &actionParams[idx];
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwSwitch::LinkCommand>( mySwitch, cmd, HmwSwitch::JT_ONDELAY );
   REQUIRE( mySwitch.getNextActionTime().isValid() == mySwitch.isValidActionTime( cmd.actionParameter->onDelayTime ) );

   // test JT_ONDELAY -> JT_ON
   REQUIRE_STATE_ENTRY_BY_LOOP( mySwitch, mySwitch.convertToTime( cmd.actionParameter->onDelayTime ), HmwSwitch::JT_ON );
   REQUIRE( mySwitch.getNextActionTime().isValid() == mySwitch.isValidActionTime( cmd.actionParameter->onTime ) );

   // test JT_ON -> JT_OFFDELAY
   if ( !mySwitch.getNextActionTime().isValid() )
   {
      cmd.keyNum++;
      REQUIRE_STATE_ENTRY_BY_ACTION<HmwSwitch::LinkCommand>( mySwitch, cmd, HmwSwitch::JT_OFFDELAY );
   }
   else
   {
      REQUIRE_STATE_ENTRY_BY_LOOP( mySwitch, mySwitch.convertToTime( cmd.actionParameter->onTime ), HmwSwitch::JT_OFFDELAY );
   }
   REQUIRE( mySwitch.getNextActionTime().isValid() == mySwitch.isValidActionTime( cmd.actionParameter->offDelayTime ) );

   // test JT_OFFDELAY -> JT_OFF
   REQUIRE_STATE_ENTRY_BY_LOOP( mySwitch, mySwitch.convertToTime( cmd.actionParameter->offDelayTime ), HmwSwitch::JT_OFF );
   REQUIRE( mySwitch.getNextActionTime().isValid() == mySwitch.isValidActionTime( cmd.actionParameter->offTime ) );

   if ( mySwitch.getNextActionTime().isValid() )
   {
      // test JT_OFF -> JT_ONDELAY
      REQUIRE_STATE_ENTRY_BY_LOOP( mySwitch, mySwitch.convertToTime( cmd.actionParameter->offTime ), HmwSwitch::JT_ONDELAY );
   }
}

TEST_CASE( "Check HmwSwitch profile On/Off behavior with additional user events", "[HmwSwitch]" )
{
   HmwSwitch& mySwitch = UnitTestDeviceHw::instance().getHmwSwitch();
   mySwitch.checkConfig();

   HmwSwitch::ActionParameter actionParams =
   {
      // 1s on delay, 1s on, 1s off delay, 1s off
      .actionType = 1,
      .notUsed = 0,
      .toggleUse = 0,
      .offTimeMode = 1,
      .onTimeMode = 1,
      .onDelayTime = 10,
      .onTime = 10,
      .offDelayTime = 10,
      .offTime = 10,
      .jtOnDelay = HmwSwitch::JT_ON,
      .jtOn = HmwSwitch::JT_OFFDELAY,
      .jtOffDelay = HmwSwitch::JT_OFF,
      .jtOff = HmwSwitch::JT_ONDELAY,
   };

   INFO( "prepare object for test" );
   uint8_t level = 0;
   mySwitch.set( sizeof( level ), &level );
   mySwitch.loop();
   REQUIRE( mySwitch.getCurrentState() == HmwSwitch::JT_OFF );

   INFO( "test JT_OFF -> JT_ONDELAY" );
   HmwSwitch::LinkCommand cmd;
   cmd.keyNum = mySwitch.getLastKeyNum() + 1;
   cmd.actionParameter = &actionParams;
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwSwitch::LinkCommand>( mySwitch, cmd, HmwSwitch::JT_ONDELAY );
   REQUIRE( mySwitch.getNextActionTime().isValid() );

   INFO( "test JT_ONDELAY -> JT_ON" );
   cmd.keyNum++;
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwSwitch::LinkCommand>( mySwitch, cmd, HmwSwitch::JT_ON, 50 );
   REQUIRE( mySwitch.getNextActionTime().isValid() );

   INFO( "test JT_ON -> JT_OFFDELAY" );
   cmd.keyNum++;
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwSwitch::LinkCommand>( mySwitch, cmd, HmwSwitch::JT_OFFDELAY, 50 );
   REQUIRE( mySwitch.getNextActionTime().isValid() );

   INFO( "test JT_OFFDELAY -> JT_OFF" );
   cmd.keyNum++;
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwSwitch::LinkCommand>( mySwitch, cmd, HmwSwitch::JT_OFF, 50 );
   REQUIRE( mySwitch.getNextActionTime().isValid() );

   INFO( "test JT_OFF -> JT_ONDELAY" );
   cmd.keyNum++;
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwSwitch::LinkCommand>( mySwitch, cmd, HmwSwitch::JT_ONDELAY, 50 );
   REQUIRE( mySwitch.getNextActionTime().isValid() );
}

TEST_CASE( "Check HmwSwitch profile On", "[HmwSwitch]" )
{
   HmwSwitch& mySwitch = UnitTestDeviceHw::instance().getHmwSwitch();
   mySwitch.checkConfig();

   HmwSwitch::ActionParameter actionParams =
   {
      // no on delay, 200ms on, no off delay, endless off
      .actionType = 1,
      .notUsed = 0,
      .toggleUse = 0,
      .offTimeMode = 1,
      .onTimeMode = 1,
      .onDelayTime = 0,
      .onTime = 4,
      .offDelayTime = 0,
      .offTime = 0xC000,
      .jtOnDelay = HmwSwitch::JT_ON,
      .jtOn = HmwSwitch::JT_ON,
      .jtOffDelay = HmwSwitch::JT_ON,
      .jtOff = HmwSwitch::JT_ONDELAY,
   };

   INFO( "prepare object for test" );
   uint8_t level = 0;
   mySwitch.set( sizeof( level ), &level );
   mySwitch.loop();
   REQUIRE( mySwitch.getCurrentState() == HmwSwitch::JT_OFF );

   INFO( "test JT_OFF -> JT_ONDELAY" );
   HmwSwitch::LinkCommand cmd;
   cmd.keyNum = mySwitch.getLastKeyNum() + 1;
   cmd.actionParameter = &actionParams;
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwSwitch::LinkCommand>( mySwitch, cmd, HmwSwitch::JT_ONDELAY );
   REQUIRE( mySwitch.getNextActionTime().isValid() == true );

   INFO( "test JT_ONDELAY -> JT_ON" );
   REQUIRE_STATE_ENTRY_BY_LOOP( mySwitch, mySwitch.convertToTime( cmd.actionParameter->onDelayTime ), HmwSwitch::JT_ON );
   REQUIRE( mySwitch.getNextActionTime().isValid() == true );

   SECTION( "one time action" )
   {
   }

   SECTION( "retriggered action" )
   {
      INFO( "wait until half of the period has past and retrigger" );
      SystemTime::waitMs( mySwitch.convertToTime( cmd.actionParameter->onTime ) / 2 );

      INFO( "test JT_ON -> JT_ON" );
      cmd.keyNum++;  // must be another keynum
      REQUIRE_STATE_ENTRY_BY_ACTION<HmwSwitch::LinkCommand>( mySwitch, cmd, HmwSwitch::JT_ON );
      REQUIRE( mySwitch.getNextActionTime().isValid() == true );
   }
   INFO( "test JT_ON -> JT_OFF" );
   REQUIRE_STATE_ENTRY_BY_LOOP( mySwitch, mySwitch.convertToTime( cmd.actionParameter->onTime ), HmwSwitch::JT_OFF );
   REQUIRE( mySwitch.getNextActionTime().isValid() == false );
}

TEST_CASE( "Check HmwSwitch profile Off", "[HmwSwitch]" )
{
   HmwSwitch& mySwitch = UnitTestDeviceHw::instance().getHmwSwitch();
   mySwitch.checkConfig();

   HmwSwitch::ActionParameter actionParams[] =
   {
      {
         // no off delay, endless off
         .actionType = HmwSwitch::ACTIVE,
         .notUsed = 0,
         .toggleUse = HmwSwitch::DONT_USE,
         .offTimeMode = HmwSwitch::TIME_MODE_MINIMAL,
         .onTimeMode = HmwSwitch::TIME_MODE_MINIMAL,
         .onDelayTime = 0,
         .onTime = HmwSwitch::ENDLESS_TIME,
         .offDelayTime = 0,
         .offTime = HmwSwitch::ENDLESS_TIME,
         .jtOnDelay = HmwSwitch::JT_OFF,
         .jtOn = HmwSwitch::JT_OFFDELAY,
         .jtOffDelay = HmwSwitch::JT_OFF,
         .jtOff = HmwSwitch::JT_OFF,
      },
      {
         // 300ms off delay, endless off
         .actionType = HmwSwitch::ACTIVE,
         .notUsed = 0,
         .toggleUse = HmwSwitch::DONT_USE,
         .offTimeMode = HmwSwitch::TIME_MODE_MINIMAL,
         .onTimeMode = HmwSwitch::TIME_MODE_MINIMAL,
         .onDelayTime = 0,
         .onTime = HmwSwitch::ENDLESS_TIME,
         .offDelayTime = 3,
         .offTime = HmwSwitch::ENDLESS_TIME,
         .jtOnDelay = HmwSwitch::JT_OFF,
         .jtOn = HmwSwitch::JT_OFFDELAY,
         .jtOffDelay = HmwSwitch::JT_OFF,
         .jtOff = HmwSwitch::JT_OFF,
      },
      {
         // 300ms off delay, 200ms off
         .actionType = HmwSwitch::ACTIVE,
         .notUsed = 0,
         .toggleUse = HmwSwitch::DONT_USE,
         .offTimeMode = HmwSwitch::TIME_MODE_ABSOLUTE,
         .onTimeMode = HmwSwitch::TIME_MODE_MINIMAL,
         .onDelayTime = 0,
         .onTime = HmwSwitch::ENDLESS_TIME,
         .offDelayTime = 3,
         .offTime = 2,
         .jtOnDelay = HmwSwitch::JT_OFF,
         .jtOn = HmwSwitch::JT_OFFDELAY,
         .jtOffDelay = HmwSwitch::JT_OFF,
         .jtOff = HmwSwitch::JT_OFF,
      },
   };
   const int parameterSize = sizeof( actionParams ) / sizeof( HmwSwitch::ActionParameter );

   // select the parameter set
   auto idx = GENERATE( range( 0, parameterSize ) );
   CAPTURE( idx );

   INFO( "prepare object for test -> ON" );
   uint8_t level = HmwSwitch::MAX_LEVEL;
   mySwitch.set( sizeof( level ), &level );
   mySwitch.loop();
   REQUIRE( mySwitch.getCurrentState() == HmwSwitch::JT_ON );

   INFO( "test JT_ON -> JT_OFFDELAY" );
   HmwSwitch::LinkCommand cmd;
   cmd.keyNum = mySwitch.getLastKeyNum() + 1;
   cmd.actionParameter = &actionParams[idx];
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwSwitch::LinkCommand>( mySwitch, cmd, HmwSwitch::JT_OFFDELAY );
   REQUIRE( mySwitch.getNextActionTime().isValid() == mySwitch.isValidActionTime( cmd.actionParameter->offDelayTime ) );

   INFO( "test JT_OFFDELAY -> JT_OFF" );
   SECTION( "one time action" )
   {
      REQUIRE_STATE_ENTRY_BY_LOOP( mySwitch, mySwitch.convertToTime( cmd.actionParameter->offDelayTime ), HmwSwitch::JT_OFF );
      REQUIRE( mySwitch.getNextActionTime().isValid() == mySwitch.isValidActionTime( cmd.actionParameter->offTime ) );

      if ( mySwitch.getNextActionTime().isValid() )
      {
         INFO( "test JT_OFF -> JT_ON" );
         REQUIRE_STATE_ENTRY_BY_LOOP( mySwitch, mySwitch.convertToTime( cmd.actionParameter->offTime ), HmwSwitch::JT_ON );
      }
      else
      {
         INFO( "test JT_OFF -> JT_OFF" );
         REQUIRE_STATE_ENTRY_BY_LOOP( mySwitch, mySwitch.convertToTime( cmd.actionParameter->offTime ), HmwSwitch::JT_OFF );
      }
   }

   SECTION( "retriggered action" )
   {
      INFO( "wait until half of the offDelayTime has past and retrigger" );
      SystemTime::waitMs( mySwitch.convertToTime( cmd.actionParameter->offDelayTime ) / 2 );

      cmd.keyNum++;  // must be another keynum
      REQUIRE_STATE_ENTRY_BY_ACTION<HmwSwitch::LinkCommand>( mySwitch, cmd, HmwSwitch::JT_OFF );
      REQUIRE( mySwitch.getNextActionTime().isValid() == mySwitch.isValidActionTime( cmd.actionParameter->offTime ) );

      if ( mySwitch.getNextActionTime().isValid() )
      {
         INFO( "wait until half of the offTime has past and retrigger" );
         SystemTime::waitMs( mySwitch.convertToTime( cmd.actionParameter->offTime ) / 2 );

         INFO( "test JT_OFF -> JT_ONDELAY" );
         cmd.keyNum++;  // must be another keynum
         REQUIRE_STATE_ENTRY_BY_ACTION<HmwSwitch::LinkCommand>( mySwitch, cmd, HmwSwitch::JT_ONDELAY );

         INFO( "test JT_ONDELAY -> JT_ON" );
         REQUIRE_STATE_ENTRY_BY_LOOP( mySwitch, 0, HmwSwitch::JT_ON );
      }
      else
      {
         INFO( "test JT_OFF -> JT_OFF" );
         REQUIRE_STATE_ENTRY_BY_LOOP( mySwitch, mySwitch.convertToTime( cmd.actionParameter->offTime ), HmwSwitch::JT_OFF );
      }
   }
   REQUIRE( mySwitch.getNextActionTime().isValid() == false );
}
