/*
 * HmwMultiKeySD6BaseHw.cpp
 *
 * Created: 09.01.2018 18:36:42
 * Author: viktor.pankraz
 */


#include "HBWLcSw16Hw.h"

// this is the EEPROM layout used by one device
struct hbw_config
{
   HmwDeviceHw::BasicConfig basicConfig;  // 0x0000 - 0x0009
   uint8_tx reserved[6];                  // 0x000A - 0x000F
   HmwSwitch::Config switchCfg[16];       // 0x0010 - 0x002F
   HmwKey::Config keycfg[10];             // 0x0030 - 0x0043
   HmwDS1820::Config ds1820cfg[6];        // 0x0044 - 0x0067
   HmwLinkSwitch::Config switchLinks[48]; // 0x0068 - 0x05A7
   HmwLinkKey::Config keyLinks[48];       // 0x05A8 - 0x06C7
                                          // 0x06C8 - 0x07FF
};

static hbw_config& config = *reinterpret_cast<hbw_config*>( MAPPED_EEPROM_START );

// default constructor
HBWLcSw16Hw::HBWLcSw16Hw() :

   hbwSwitch1( PortPin( PortA, 3 ), &config.switchCfg[0] ),
   hbwSwitch2( PortPin( PortC, 3 ), &config.switchCfg[1] ),
   hbwSwitch3( PortPin( PortA, 2 ), &config.switchCfg[2] ),
   hbwSwitch4( PortPin( PortC, 2 ), &config.switchCfg[3] ),
   hbwSwitch5( PortPin( PortA, 1 ), &config.switchCfg[4] ),
   hbwSwitch6( PortPin( PortC, 1 ), &config.switchCfg[5] ),
   hbwSwitch7( PortPin( PortA, 0 ), &config.switchCfg[6] ),
   hbwSwitch8( PortPin( PortC, 0 ), &config.switchCfg[7] ),
   hbwSwitch9( PortPin( PortD, 3 ), &config.switchCfg[8] ),
   hbwSwitch10( PortPin( PortB, 3 ), &config.switchCfg[9] ),
   hbwSwitch11( PortPin( PortD, 2 ), &config.switchCfg[10] ),
   hbwSwitch12( PortPin( PortB, 2 ), &config.switchCfg[11] ),
   hbwSwitch13( PortPin( PortD, 1 ), &config.switchCfg[12] ),
   hbwSwitch14( PortPin( PortB, 1 ), &config.switchCfg[13] ),
   hbwSwitch15( PortPin( PortD, 0 ), &config.switchCfg[14] ),
   hbwSwitch16( PortPin( PortB, 0 ), &config.switchCfg[15] ),

#ifdef _DEBUG_
   // if debugging PORTF is used for tracing
   hbwKey1( PortPin( PortDummy, 7 ), &( config.keycfg[0] ) ),
   hbwKey2( PortPin( PortDummy, 6 ), &( config.keycfg[1] ) ),
   hbwKey3( PortPin( PortDummy, 5 ), &( config.keycfg[2] ) ),
   hbwKey4( PortPin( PortDummy, 4 ), &( config.keycfg[3] ) ),
   hbwKey5( PortPin( PortDummy, 3 ), &( config.keycfg[4] ) ),
   hbwKey6( PortPin( PortDummy, 2 ), &( config.keycfg[5] ) ),
   hbwKey7( PortPin( PortDummy, 1 ), &( config.keycfg[6] ) ),
   hbwKey8( PortPin( PortDummy, 0 ), &( config.keycfg[7] ) ),
#else
   hbwKey1( PortPin( PortF, 7 ), &( config.keycfg[0] ) ),
   hbwKey2( PortPin( PortF, 6 ), &( config.keycfg[1] ) ),
   hbwKey3( PortPin( PortF, 5 ), &( config.keycfg[2] ) ),
   hbwKey4( PortPin( PortF, 4 ), &( config.keycfg[3] ) ),
   hbwKey5( PortPin( PortF, 3 ), &( config.keycfg[4] ) ),
   hbwKey6( PortPin( PortF, 2 ), &( config.keycfg[5] ) ),
   hbwKey7( PortPin( PortF, 1 ), &( config.keycfg[6] ) ),
   hbwKey8( PortPin( PortF, 0 ), &( config.keycfg[7] ) ),
#endif
   hbwKey9( PortPin( PortE, 7 ), &( config.keycfg[8] ) ),
   hbwKey10( PortPin( PortE, 6 ), &( config.keycfg[9] ) ),

   ow( PortPin( PortE, 5 ) ),
   hbwTmp1( ow, &config.ds1820cfg[0] ),
   hbwTmp2( ow, &config.ds1820cfg[1] ),
   hbwTmp3( ow, &config.ds1820cfg[2] ),
   hbwTmp4( ow, &config.ds1820cfg[3] ),
   hbwTmp5( ow, &config.ds1820cfg[4] ),
   hbwTmp6( ow, &config.ds1820cfg[5] ),

   linkReceiver( sizeof( config.switchLinks ) / sizeof( config.switchLinks[0] ), config.switchLinks ),
   linkSender( sizeof( config.keyLinks ) / sizeof( config.keyLinks[0] ), config.keyLinks )

{
#ifdef _DEBUG_
   TRACE_PORT_INIT( AllPinsMask );
#endif

} // HBWLcSw16Hw


HmwDeviceHw::BasicConfig* HBWLcSw16Hw::getBasicConfig()
{
   return &config.basicConfig;
}