/*
 * HmwBlindActorTests.cpp
 *
 * Created: 03.12.2016 02:31:44
 *  Author: Viktor Pankraz
 */

#include "MainUnitTests.h"


void CHECK_DimmerHardwareState( HmwBlindActor& blActor, uint8_t level )
{
   uint8_t currentLevel;
   REQUIRE( blActor.get( &currentLevel ) == 1 ); // size of data should be 1 Byte
   REQUIRE( currentLevel == level ); // currentLevel has to be params.onLevel
}

TEST_CASE( "Check HmwBlindActor start up conditions", "[HmwBlindActor]" )
{
   HmwBlindActor& myActor = UnitTestDeviceHw::instance().getHmwBlindActor();
   uint8_t testData[128];

   for ( uint8_t i = 0; i < 2; i++ )
   {
      CHECK( myActor.getCurrentState() == HmwBlindActor::JT_OFF );
      CHECK( myActor.get( testData ) == 1 ); // size of data should be 1 Byte
      CHECK( testData[0] == 0 ); // currentLevel has to be 0
      CHECK( myActor.getNextActionTime().isValid() == false ); // no next action is expected after startup
      CHECK( myActor.getNextFeedbackTime().isValid() == false );  // no feedback expected after startup
      myActor.loop();
      SystemTime::waitMs( 1 );
   }
   CHECK( myActor.getMoveDownOutput().isSet() == false );  // outputPin should not be set in state OFF
   CHECK( myActor.getMoveUpOutput().isSet() == false );  // enablePin should not be set in state OFF
}

TEST_CASE( "Check HmwBlindActor restoring config to default", "[HmwBlindActor]" )
{
   HmwBlindActor& myActor = UnitTestDeviceHw::instance().getHmwBlindActor();
   const HmwBlindActor::Config& config = myActor.getConfig();
   myActor.checkConfig();
   CHECK( config.isLogging() == true );
   CHECK( config.changeOverDelay == +config.DEFAULT_CHANGE_OVER_DELAY );
   CHECK( config.runCounter == +config.DEFAULT_RUN_COUNTER );
   CHECK( config.runTimeBottomTop == +config.DEFAULT_RUN_TIME );
   CHECK( config.runTimeTopBottom == +config.DEFAULT_RUN_TIME );
}

TEST_CASE( "Check HmwBlindActor profile Up/Down behavior", "[HmwBlindActor]" )
{
   HmwBlindActor& myActor = UnitTestDeviceHw::instance().getHmwBlindActor();
   myActor.checkConfig();

   uint8_t level = 0;
   myActor.set( sizeof( level ), &level );
   myActor.loop();
   REQUIRE( myActor.getCurrentState() == HmwBlindActor::JT_OFF );

   level = 10;
   myActor.set( sizeof( level ), &level ); 
   REQUIRE_STATE_ENTRY_BY_LOOP( myActor, 10, HmwBlindActor::JT_RAMP_ON ); 
   REQUIRE_STATE_ENTRY_BY_LOOP( myActor, 2500, HmwBlindActor::JT_ON );
   myActor.get( &level );
   REQUIRE( level == 10 );  

   level = 20;
   myActor.set( sizeof( level ), &level ); 
   REQUIRE_STATE_ENTRY_BY_LOOP( myActor, 10, HmwBlindActor::JT_RAMP_ON );
   REQUIRE_STATE_ENTRY_BY_LOOP( myActor, 2500, HmwBlindActor::JT_ON );   
   myActor.get( &level );
   REQUIRE( level == 20 );  

   level = 10;
   myActor.set( sizeof( level ), &level ); 
   REQUIRE_STATE_ENTRY_BY_LOOP( myActor, 10, HmwBlindActor::JT_RAMP_OFF );
   REQUIRE_STATE_ENTRY_BY_LOOP( myActor, 2500, HmwBlindActor::JT_OFF ); 
   myActor.get( &level );
   REQUIRE( level == 10 ); 

   level = 0;
   myActor.set( sizeof( level ), &level ); 
   REQUIRE_STATE_ENTRY_BY_LOOP( myActor, 10, HmwBlindActor::JT_RAMP_OFF );
   REQUIRE_STATE_ENTRY_BY_LOOP( myActor, 2500, HmwBlindActor::JT_OFF ); 
   myActor.get( &level );
   REQUIRE( level == 0 );      
}

TEST_CASE( "Check HmwBlindActor profile Stop - Up/Down behavior", "[HmwBlindActor]" )
{
   HmwBlindActor& myActor = UnitTestDeviceHw::instance().getHmwBlindActor();
   myActor.checkConfig();

   uint8_t level = 10;
   myActor.set( sizeof( level ), &level ); 
   LOOP_FOR_SPECIFIC_TIME( myActor, 1000 );

   level = HmwChannel::STOP;
   myActor.set( sizeof( level ), &level ); 
   REQUIRE_STATE_ENTRY_BY_LOOP( myActor, 50, HmwBlindActor::JT_ON ); 
   myActor.get( &level );
   REQUIRE( level == 4 );

   level = 0;
   myActor.set( sizeof( level ), &level ); 
   LOOP_FOR_SPECIFIC_TIME( myActor, 500 );

   level = HmwChannel::STOP;
   myActor.set( sizeof( level ), &level ); 
   REQUIRE_STATE_ENTRY_BY_LOOP( myActor, 50, HmwBlindActor::JT_OFF );   
   myActor.get( &level );
   REQUIRE( level == 2 );    

   level = 0;
   myActor.set( sizeof( level ), &level ); 
   REQUIRE_STATE_ENTRY_BY_LOOP( myActor, 500, HmwBlindActor::JT_OFF );       
}