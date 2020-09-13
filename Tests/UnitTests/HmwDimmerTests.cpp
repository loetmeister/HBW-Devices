/*
 * HmwDimmerTests.cpp
 *
 * Created: 03.12.2016 02:31:44
 *  Author: Viktor Pankraz
 */

#include "MainUnitTests.h"

void CHECK_DimmerHardwareState( HmwDimmer& dimmer, uint8_t level )
{
   uint8_t currentLevel;
   REQUIRE( dimmer.get( &currentLevel ) == 1 ); // size of data should be 1 Byte
   REQUIRE( currentLevel == level ); // currentLevel has to be params.onLevel
   if ( level )
   {
      REQUIRE( dimmer.getEnableOutput().isSet() );
      REQUIRE( dimmer.getPwmOutput().isSet() );
   }
   else
   {
      REQUIRE( dimmer.getEnableOutput().isSet() == false );
      REQUIRE( dimmer.getPwmOutput().isSet() == false );
   }
}

TEST_CASE( "Check HmwDimmer start up conditions", "[HmwDimmer]" )
{
   HmwDimmer& myDimmer = UnitTestDeviceHw::instance().getHmwDimmer();
   uint8_t testData[128];

   for ( uint8_t i = 0; i < 2; i++ )
   {
      CHECK( myDimmer.getCurrentState() == HmwDimmer::JT_OFF );
      CHECK( myDimmer.get( testData ) == 1 ); // size of data should be 1 Byte
      CHECK( testData[0] == 0 ); // currentLevel has to be 0
      CHECK( myDimmer.getNextActionTime().isValid() == false ); // no next action is expected after startup
      CHECK( myDimmer.getNextFeedbackTime().isValid() == false );  // no feedback expected after startup
      myDimmer.loop();
      SystemTime::waitMs( 1 );
   }
   CHECK( myDimmer.getPwmOutput().isSet() == false );  // outputPin should not be set in state OFF
   CHECK( myDimmer.getEnableOutput().isSet() == false );  // enablePin should not be set in state OFF
}

TEST_CASE( "Check HmwDimmer restoring config to default", "[HmwDimmer]" )
{
   HmwDimmer& myDimmer = UnitTestDeviceHw::instance().getHmwDimmer();
   const HmwDimmer::Config& config = myDimmer.getConfig();
   myDimmer.checkConfig();
   CHECK( config.isLogging() == true );
   CHECK( config.isDimmingModeTrailing() == true );
   CHECK( config.getPwmRangeStart() == HmwDimmer::Config::_00_PERCENT );
   CHECK( config.getPwmRangeEnd() == HmwDimmer::Config::_100_PERCENT );
}

TEST_CASE( "Check HmwDimmer profile On/Off behavior", "[HmwDimmer]" )
{
   HmwDimmer& myDimmer = UnitTestDeviceHw::instance().getHmwDimmer();
   myDimmer.checkConfig();

   HmwDimmer::ActionParameter shortActionParams[] =
   {
      // no on delay, no rampOn, endless on, no off delay, no rampDown, endless off
      // for longPress only: dimLevelOff 0%, dimLevelOn 100%
      {
         .actionType = HmwDimmer::JUMP_TO_TARGET,
         .multiExecute = 1,
         .onDelayMode = 0,
         .offTimeMode = 0,
         .onTimeMode = 0,
         .offLevel = 0,
         .onMinLevel = 20,
         .onLevel = 200,
         .rampStartStep = 10,
         .offDelayStep = 10,
         .onDelayTime = 0,
         .rampOnTime = 0,
         .onTime = 0xC000,
         .offDelayTime = 0,
         .rampOffTime = 0,
         .offTime = 0xC000,
         .dimMinLevel = 0,
         .dimMaxLevel = 200,
         .dimStep = 10,
         .jtOnDelay = HmwDimmer::JT_RAMP_ON,
         .jtRampOn = HmwDimmer::JT_ON,
         .jtOn = HmwDimmer::JT_OFFDELAY,
         .jtOffDelay = HmwDimmer::JT_RAMP_OFF,
         .jtRampOff = HmwDimmer::JT_OFF,
         .jtOff = HmwDimmer::JT_ONDELAY,
      },
      // 100ms on delay, 200ms rampOn, endless on level 100, 300ms off delay, 200ms rampDown, endless off
      // for longPress only: dimLevelOff 0%, dimLevelOn 100%
      {
         .actionType = HmwDimmer::JUMP_TO_TARGET,
         .multiExecute = 1,
         .onDelayMode = 0,
         .offTimeMode = 0,
         .onTimeMode = 0,
         .offLevel = 0,
         .onMinLevel = 20,
         .onLevel = 100,
         .rampStartStep = 10,
         .offDelayStep = 10,
         .onDelayTime = 1,
         .rampOnTime = 2,
         .onTime = 0xC000,
         .offDelayTime = 3,
         .rampOffTime = 2,
         .offTime = 0xC000,
         .dimMinLevel = 0,
         .dimMaxLevel = 200,
         .dimStep = 10,
         .jtOnDelay = HmwDimmer::JT_RAMP_ON,
         .jtRampOn = HmwDimmer::JT_ON,
         .jtOn = HmwDimmer::JT_OFFDELAY,
         .jtOffDelay = HmwDimmer::JT_RAMP_OFF,
         .jtRampOff = HmwDimmer::JT_OFF,
         .jtOff = HmwDimmer::JT_ONDELAY,
      },
      // no on delay, 100ms rampOn, 200ms on, no off delay, 500ms rampDown, endless off
      // for longPress only: dimLevelOff 0%, dimLevelOn 100%
      {
         .actionType = HmwDimmer::JUMP_TO_TARGET,
         .multiExecute = 1,
         .onDelayMode = 0,
         .offTimeMode = 0,
         .onTimeMode = 0,
         .offLevel = 0,
         .onMinLevel = 20,
         .onLevel = 200,
         .rampStartStep = 10,
         .offDelayStep = 10,
         .onDelayTime = 0,
         .rampOnTime = 1,
         .onTime = 2,
         .offDelayTime = 0,
         .rampOffTime = 5,
         .offTime = 0xC000,
         .dimMinLevel = 0,
         .dimMaxLevel = 200,
         .dimStep = 10,
         .jtOnDelay = HmwDimmer::JT_RAMP_ON,
         .jtRampOn = HmwDimmer::JT_ON,
         .jtOn = HmwDimmer::JT_OFFDELAY,
         .jtOffDelay = HmwDimmer::JT_RAMP_OFF,
         .jtRampOff = HmwDimmer::JT_OFF,
         .jtOff = HmwDimmer::JT_ONDELAY,
      },

   };

   const int parameterSize = sizeof( shortActionParams ) / sizeof( HmwDimmer::ActionParameter );

   // select the parameter set
   auto idx = GENERATE( range( 0, parameterSize ) );
   CAPTURE( idx );

   INFO( "prepare object" );
   uint8_t level = 0;
   myDimmer.set( sizeof( level ), &level );
   myDimmer.loop();
   REQUIRE( myDimmer.getCurrentState() == HmwDimmer::JT_OFF );

   INFO( "JT_OFF -> JT_ONDELAY" );
   HmwDimmer::LinkCommand cmd;
   cmd.keyNum = myDimmer.getLastKeyNum() + 1;
   cmd.actionParameter = &shortActionParams[idx];
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwDimmer::LinkCommand>( myDimmer, cmd, HmwDimmer::JT_ONDELAY );
   REQUIRE( myDimmer.getNextActionTime().isValid() == myDimmer.isValidActionTime( cmd.actionParameter->onDelayTime ) );

   INFO( "JT_ONDELAY -> JT_RAMP_ON" );
   REQUIRE_STATE_ENTRY_BY_LOOP( myDimmer, myDimmer.convertToTime( cmd.actionParameter->onDelayTime ), HmwDimmer::JT_RAMP_ON );
   REQUIRE( myDimmer.getNextActionTime().isValid() == myDimmer.isValidActionTime( cmd.actionParameter->rampOnTime ) );

   INFO( "JT_RAMP_ON -> JT_ON" );
   REQUIRE_STATE_ENTRY_BY_LOOP( myDimmer, myDimmer.convertToTime( cmd.actionParameter->rampOnTime ), HmwDimmer::JT_ON );
   REQUIRE( myDimmer.getNextActionTime().isValid() == myDimmer.isValidActionTime( cmd.actionParameter->onTime ) );

   CHECK_DimmerHardwareState( myDimmer, cmd.actionParameter->onLevel );

   INFO( "JT_ON -> JT_OFFDELAY " );
   if ( !myDimmer.getNextActionTime().isValid() )
   {
      INFO( "by action" );
      cmd.keyNum++;
      REQUIRE_STATE_ENTRY_BY_ACTION<HmwDimmer::LinkCommand>( myDimmer, cmd, HmwDimmer::JT_OFFDELAY );
   }
   else
   {
      INFO( "by loop" );
      REQUIRE_STATE_ENTRY_BY_LOOP( myDimmer, myDimmer.convertToTime( cmd.actionParameter->onTime ), HmwDimmer::JT_OFFDELAY );
   }
   REQUIRE( myDimmer.getNextActionTime().isValid() == myDimmer.isValidActionTime( cmd.actionParameter->offDelayTime ) );

   INFO( "JT_OFFDELAY -> JT_RAMP_OFF" );
   REQUIRE_STATE_ENTRY_BY_LOOP( myDimmer, myDimmer.convertToTime( cmd.actionParameter->offDelayTime ), HmwDimmer::JT_RAMP_OFF );
   REQUIRE( myDimmer.getNextActionTime().isValid() == myDimmer.isValidActionTime( cmd.actionParameter->rampOffTime ) );

   INFO( "JT_RAMP_OFF -> JT_OFF" );
   REQUIRE_STATE_ENTRY_BY_LOOP( myDimmer, myDimmer.convertToTime( cmd.actionParameter->rampOffTime ), HmwDimmer::JT_OFF );
   REQUIRE( myDimmer.getNextActionTime().isValid() == myDimmer.isValidActionTime( cmd.actionParameter->offTime ) );

   CHECK_DimmerHardwareState( myDimmer, cmd.actionParameter->offLevel );

   if ( myDimmer.getNextActionTime().isValid() )
   {
      INFO( "JT_OFF -> JT_ONDELAY" );
      REQUIRE_STATE_ENTRY_BY_LOOP( myDimmer, myDimmer.convertToTime( cmd.actionParameter->offTime ), HmwDimmer::JT_ONDELAY );
   }
}

TEST_CASE( "Check HmwDimmer profile On/Off behavior with additional user events", "[HmwDimmer]" )
{
   HmwDimmer& myDimmer = UnitTestDeviceHw::instance().getHmwDimmer();
   myDimmer.checkConfig();

   HmwDimmer::ActionParameter shortActionParams =
   {
      // 1s on delay, 1s rampOn, 1s on, 1s off delay, 1s rampDown, 1s off
      // for longPress only: dimLevelOff 0%, dimLevelOn 100%
      .actionType = HmwDimmer::JUMP_TO_TARGET,
      .multiExecute = 1,
      .onDelayMode = 0,
      .offTimeMode = 0,
      .onTimeMode = 0,
      .offLevel = 0,
      .onMinLevel = 20,
      .onLevel = 200,
      .rampStartStep = 10,
      .offDelayStep = 10,
      .onDelayTime = 10,
      .rampOnTime = 10,
      .onTime = 10,
      .offDelayTime = 10,
      .rampOffTime = 10,
      .offTime = 10,
      .dimMinLevel = 0,
      .dimMaxLevel = 200,
      .dimStep = 10,
      .jtOnDelay = HmwDimmer::JT_RAMP_ON,
      .jtRampOn = HmwDimmer::JT_ON,
      .jtOn = HmwDimmer::JT_OFFDELAY,
      .jtOffDelay = HmwDimmer::JT_RAMP_OFF,
      .jtRampOff = HmwDimmer::JT_OFF,
      .jtOff = HmwDimmer::JT_ONDELAY,
   };

   INFO( "prepare object" );
   uint8_t level = 0;
   myDimmer.set( sizeof( level ), &level );
   myDimmer.loop();
   REQUIRE( myDimmer.getCurrentState() == HmwDimmer::JT_OFF );

   INFO( "JT_OFF -> JT_ONDELAY" );
   HmwDimmer::LinkCommand cmd;
   cmd.keyNum = myDimmer.getLastKeyNum() + 1;
   cmd.actionParameter = &shortActionParams;
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwDimmer::LinkCommand>( myDimmer, cmd, HmwDimmer::JT_ONDELAY );
   REQUIRE( myDimmer.getNextActionTime().isValid() );

   INFO( "JT_ONDELAY -> JT_RAMP_ON" );
   cmd.keyNum++;
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwDimmer::LinkCommand>( myDimmer, cmd, HmwDimmer::JT_RAMP_ON, 50 );
   REQUIRE( myDimmer.getNextActionTime().isValid() );

   INFO( "JT_RAMP_ON -> JT_ON" );
   cmd.keyNum++;
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwDimmer::LinkCommand>( myDimmer, cmd, HmwDimmer::JT_ON, 50 );
   REQUIRE( myDimmer.getNextActionTime().isValid() );

   INFO( "JT_ON -> JT_OFFDELAY " );
   cmd.keyNum++;
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwDimmer::LinkCommand>( myDimmer, cmd, HmwDimmer::JT_OFFDELAY, 50 );
   REQUIRE( myDimmer.getNextActionTime().isValid() );

   INFO( "JT_OFFDELAY -> JT_RAMP_OFF" );
   cmd.keyNum++;
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwDimmer::LinkCommand>( myDimmer, cmd, HmwDimmer::JT_RAMP_OFF, 50 );
   REQUIRE( myDimmer.getNextActionTime().isValid() );

   INFO( "JT_RAMP_OFF -> JT_OFF" );
   cmd.keyNum++;
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwDimmer::LinkCommand>( myDimmer, cmd, HmwDimmer::JT_OFF, 50 );
   REQUIRE( myDimmer.getNextActionTime().isValid() );

   INFO( "JT_OFF -> JT_ONDELAY" );
   cmd.keyNum++;
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwDimmer::LinkCommand>( myDimmer, cmd, HmwDimmer::JT_ONDELAY, 50 );
   REQUIRE( myDimmer.getNextActionTime().isValid() );
}

TEST_CASE( "Check HmwDimmer profile On/Off behavior with dimUp/dimDown", "[HmwDimmer]" )
{
   HmwDimmer& myDimmer = UnitTestDeviceHw::instance().getHmwDimmer();
   myDimmer.checkConfig();

   HmwDimmer::ActionParameter longActionParams =
   {
      // 1s on delay, 1s rampOn, 1s on, 1s off delay, 1s rampDown, 1s off
      // for longPress only: dimLevelOff 0%, dimLevelOn 100%
      .actionType = HmwDimmer::TOGGLEDIM,
      .multiExecute = 1,
      .onDelayMode = 0,
      .offTimeMode = 0,
      .onTimeMode = 0,
      .offLevel = 0,
      .onMinLevel = 20,
      .onLevel = 200,
      .rampStartStep = 10,
      .offDelayStep = 0,
      .onDelayTime = 0,
      .rampOnTime = 0,
      .onTime = HmwDimmer::ENDLESS_TIME,
      .offDelayTime = 0,
      .rampOffTime = 0,
      .offTime = HmwDimmer::ENDLESS_TIME,
      .dimMinLevel = 20,
      .dimMaxLevel = 160,
      .dimStep = 10,
      .jtOnDelay = HmwDimmer::JT_RAMP_ON,
      .jtRampOn = HmwDimmer::JT_ON,
      .jtOn = HmwDimmer::JT_OFFDELAY,
      .jtOffDelay = HmwDimmer::JT_RAMP_OFF,
      .jtRampOff = HmwDimmer::JT_OFF,
      .jtOff = HmwDimmer::JT_ONDELAY,
   };

   INFO( "prepare object" );
   uint8_t level = 0;
   myDimmer.set( sizeof( level ), &level );
   myDimmer.loop();
   REQUIRE( myDimmer.getCurrentState() == HmwDimmer::JT_OFF );

   INFO( "JT_OFF -> DIM_UP" );
   HmwDimmer::LinkCommand cmd;
   cmd.keyNum = ( myDimmer.getLastKeyNum() + 2 ) | 1; // last bit must be set to DIM_UP
   cmd.actionParameter = &longActionParams;
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwDimmer::LinkCommand>( myDimmer, cmd, HmwDimmer::DIM_UP );
   REQUIRE( myDimmer.getNextActionTime().isValid() );

   INFO( "1st DIM_UP -> JT_ON" );
   REQUIRE_STATE_ENTRY_BY_LOOP( myDimmer, HmwDimmer::MAX_DIMMING_TIME, HmwDimmer::JT_ON );
   REQUIRE( myDimmer.getNextActionTime().isValid() == false );
   CHECK_DimmerHardwareState( myDimmer, cmd.actionParameter->dimMinLevel );

   INFO( "JT_ON -> DIM_UP" );
   // repeat key event with same keyNum to simulate longPress
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwDimmer::LinkCommand>( myDimmer, cmd, HmwDimmer::DIM_UP );
   REQUIRE( myDimmer.getNextActionTime().isValid() );

   INFO( "2nd DIM_UP -> JT_ON" );
   REQUIRE_STATE_ENTRY_BY_LOOP( myDimmer, HmwDimmer::MAX_DIMMING_TIME, HmwDimmer::JT_ON );
   REQUIRE( myDimmer.getNextActionTime().isValid() == false );
   CHECK_DimmerHardwareState( myDimmer, cmd.actionParameter->dimMinLevel + cmd.actionParameter->dimStep );

   INFO( "set dimMaxLevel - dimStep/2" );
   level = cmd.actionParameter->dimMaxLevel - cmd.actionParameter->dimStep / 2;
   myDimmer.set( sizeof( level ), &level );
   myDimmer.loop();
   REQUIRE( myDimmer.getCurrentState() == HmwDimmer::JT_ON );

   INFO( "JT_ON -> DIM_UP" );
   // repeat key event with same keyNum to simulate longPress
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwDimmer::LinkCommand>( myDimmer, cmd, HmwDimmer::DIM_UP );
   REQUIRE( myDimmer.getNextActionTime().isValid() );

   INFO( "3rd DIM_UP -> JT_ON" );
   REQUIRE_STATE_ENTRY_BY_LOOP( myDimmer, HmwDimmer::MAX_DIMMING_TIME, HmwDimmer::JT_ON );
   REQUIRE( myDimmer.getNextActionTime().isValid() == false );
   CHECK_DimmerHardwareState( myDimmer, cmd.actionParameter->dimMaxLevel );

   INFO( "JT_ON -> DIM_DOWN with new key event" );
   cmd.keyNum++;
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwDimmer::LinkCommand>( myDimmer, cmd, HmwDimmer::DIM_DOWN );
   REQUIRE( myDimmer.getNextActionTime().isValid() );

   INFO( "1st DIM_DOWN -> JT_ON" );
   REQUIRE_STATE_ENTRY_BY_LOOP( myDimmer, HmwDimmer::MAX_DIMMING_TIME, HmwDimmer::JT_ON );
   REQUIRE( myDimmer.getNextActionTime().isValid() == false );
   CHECK_DimmerHardwareState( myDimmer, cmd.actionParameter->dimMaxLevel - cmd.actionParameter->dimStep );

   INFO( "JT_ON -> DIM_DOWN with longPress" );
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwDimmer::LinkCommand>( myDimmer, cmd, HmwDimmer::DIM_DOWN );
   REQUIRE( myDimmer.getNextActionTime().isValid() );

   INFO( "2nd DIM_DOWN -> JT_ON" );
   REQUIRE_STATE_ENTRY_BY_LOOP( myDimmer, HmwDimmer::MAX_DIMMING_TIME, HmwDimmer::JT_ON );
   REQUIRE( myDimmer.getNextActionTime().isValid() == false );
   CHECK_DimmerHardwareState( myDimmer, cmd.actionParameter->dimMaxLevel - 2 * cmd.actionParameter->dimStep );

   INFO( "set dimMinLevel + dimStep/2" );
   level = cmd.actionParameter->dimMinLevel + cmd.actionParameter->dimStep / 2;
   myDimmer.set( sizeof( level ), &level );
   myDimmer.loop();
   CHECK_DimmerHardwareState( myDimmer, level );

   INFO( "JT_ON -> DIM_DOWN" );
   // repeat key event with same keyNum to simulate longPress
   REQUIRE_STATE_ENTRY_BY_ACTION<HmwDimmer::LinkCommand>( myDimmer, cmd, HmwDimmer::DIM_DOWN );
   REQUIRE( myDimmer.getNextActionTime().isValid() );

   INFO( "3rd DIM_DOWN -> JT_OFF" );

   REQUIRE_STATE_ENTRY_BY_LOOP( myDimmer, HmwDimmer::MAX_DIMMING_TIME, cmd.actionParameter->dimMinLevel ? HmwDimmer::JT_ON : HmwDimmer::JT_OFF );
   REQUIRE( myDimmer.getNextActionTime().isValid() == false );
   CHECK_DimmerHardwareState( myDimmer, cmd.actionParameter->dimMinLevel );
}