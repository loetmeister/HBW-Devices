/*
 * HmwKeyTests.cpp
 *
 * Created: 03.12.2016 02:31:44
 *  Author: Viktor Pankraz
 */

#include "MainUnitTests.h"


TEST_CASE( "Check HmwKey start up conditions", "[HmwKey]" )
{
   HmwKey& myKey = UnitTestDeviceHw::instance().getHmwKey();
   CHECK( myKey.getDigitalInput().isSet() );
}

TEST_CASE( "Check HmwKey restoring config to default", "[HmwKey]" )
{
   HmwKey& myKey = UnitTestDeviceHw::instance().getHmwKey();
   const HmwKey::Config& config = myKey.getConfig();
   myKey.checkConfig();
   CHECK( config.isUnlocked() == true );
   CHECK( config.isActiveLow() == true );
   CHECK( config.isPushButton() == true );
   CHECK( config.getLongPressTime() == config.DEFAULT_LONG_PRESS_TIME );
}