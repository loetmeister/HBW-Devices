/*
 * HmwMultiKeySD6BaseHw.cpp
 *
 * Created: 09.01.2018 18:36:42
 * Author: viktor.pankraz
 */


#include "HBWLcSw8Hw.h"

// this is the EEPROM layout used by one device
struct hbw_config
{
   HmwDeviceHw::BasicConfig basicConfig;  // 0x0000 - 0x0009
   uint8_tx reserved[6];                  // 0x000A - 0x000F
   HmwSwitch::Config switchCfg[8];        // 0x0010 - 0x001F
   HmwKey::Config keycfg[8];              // 0x0020 - 0x002F
   HmwLinkSwitch::Config switchLinks[32]; // 0x0030 - 0x03AF
   HmwLinkKey::Config keyLinks[32];       // 0x03B0 - 0x046F
                                          // 0x0470 - 0x07FF
};

static hbw_config& config = *reinterpret_cast<hbw_config*>( MAPPED_EEPROM_START );

// default constructor
HBWLcSw8Hw::HBWLcSw8Hw() :

   hbwSwitch1( PortPin( PortA, 3 ), &config.switchCfg[0] ),
   hbwSwitch2( PortPin( PortA, 2 ), &config.switchCfg[1] ),
   hbwSwitch3( PortPin( PortA, 1 ), &config.switchCfg[2] ),
   hbwSwitch4( PortPin( PortA, 0 ), &config.switchCfg[3] ),
   hbwSwitch5( PortPin( PortB, 3 ), &config.switchCfg[4] ),
   hbwSwitch6( PortPin( PortB, 2 ), &config.switchCfg[5] ),
   hbwSwitch7( PortPin( PortB, 1 ), &config.switchCfg[6] ),
   hbwSwitch8( PortPin( PortB, 0 ), &config.switchCfg[7] ),

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
   hbwKey1( PortPin( PortF, 1 ), &( config.keycfg[0] ) ),
   hbwKey2( PortPin( PortF, 0 ), &( config.keycfg[1] ) ),
   hbwKey3( PortPin( PortF, 3 ), &( config.keycfg[2] ) ),
   hbwKey4( PortPin( PortF, 2 ), &( config.keycfg[3] ) ),
   hbwKey5( PortPin( PortF, 5 ), &( config.keycfg[4] ) ),
   hbwKey6( PortPin( PortF, 4 ), &( config.keycfg[5] ) ),
   hbwKey7( PortPin( PortF, 7 ), &( config.keycfg[6] ) ),
   hbwKey8( PortPin( PortF, 6 ), &( config.keycfg[7] ) ),
#endif

   iKey1( PortPin( PortC, 3 ), &hbwSwitch1 ),
   iKey2( PortPin( PortC, 2 ), &hbwSwitch2 ),
   iKey3( PortPin( PortC, 1 ), &hbwSwitch3 ),
   iKey4( PortPin( PortC, 0 ), &hbwSwitch4 ),
   iKey5( PortPin( PortD, 3 ), &hbwSwitch5 ),
   iKey6( PortPin( PortD, 2 ), &hbwSwitch6 ),
   iKey7( PortPin( PortD, 1 ), &hbwSwitch7 ),
   iKey8( PortPin( PortD, 0 ), &hbwSwitch8 ),

   linkReceiver( sizeof( config.switchLinks ) / sizeof( config.switchLinks[0] ), config.switchLinks ),
   linkSender( sizeof( config.keyLinks ) / sizeof( config.keyLinks[0] ), config.keyLinks )

{
#ifdef _DEBUG_
   TRACE_PORT_INIT( AllPinsMask );
#endif

} // HBWLcSw8Hw


HmwDeviceHw::BasicConfig* HBWLcSw8Hw::getBasicConfig()
{
   return &config.basicConfig;
}