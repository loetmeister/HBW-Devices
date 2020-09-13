/*
 * HBWLcBl8Hw.cpp
 *
 * Created: 09.01.2019 18:36:42
 * Author: viktor.pankraz
 */


#include "HBWLcBl8Hw.h"

// this is the EEPROM layout used by one device
struct hbw_config
{
   HmwDeviceHw::BasicConfig basicConfig;     // 0x0000 - 0x0009
   uint8_tx reserved[6];                     // 0x000A - 0x000F
   HmwBlindActor::Config blindCfg[8];        // 0x0010 - 0x004F
   HmwKey::Config keycfg[10];                // 0x0050 - 0x0063
   HmwDS1820::Config ds1820cfg[6];           // 0x0064 - 0x0087
   uint8_tx reserved2[8];                    // 0x0088 - 0x008F
   HmwLinkBlindActor::Config blindLinks[40]; // 0x0090 - 0x067F
   HmwLinkKey::Config keyLinks[48];          // 0x0680 - 0x079F
                                             // 0x07A0 - 0x07FF
};

static hbw_config& config = *reinterpret_cast<hbw_config*>( MAPPED_EEPROM_START );

// default constructor
HBWLcBl8Hw::HBWLcBl8Hw() :

   hbwBlindActor1( PortPin( PortA, 3 ), PortPin( PortC, 3 ), &config.blindCfg[0] ),
   hbwBlindActor2( PortPin( PortA, 2 ), PortPin( PortC, 2 ), &config.blindCfg[1] ),
   hbwBlindActor3( PortPin( PortA, 1 ), PortPin( PortC, 1 ), &config.blindCfg[2] ),
   hbwBlindActor4( PortPin( PortA, 0 ), PortPin( PortC, 0 ), &config.blindCfg[3] ),
   hbwBlindActor5( PortPin( PortD, 3 ), PortPin( PortB, 3 ), &config.blindCfg[4] ),
   hbwBlindActor6( PortPin( PortD, 2 ), PortPin( PortB, 2 ), &config.blindCfg[5] ),
   hbwBlindActor7( PortPin( PortD, 1 ), PortPin( PortB, 1 ), &config.blindCfg[6] ),
   hbwBlindActor8( PortPin( PortD, 0 ), PortPin( PortB, 0 ), &config.blindCfg[7] ),

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

   linkReceiver( sizeof( config.blindLinks ) / sizeof( config.blindLinks[0] ), config.blindLinks ),
   linkSender( sizeof( config.keyLinks ) / sizeof( config.keyLinks[0] ), config.keyLinks )

{
#ifdef _DEBUG_
   TRACE_PORT_INIT( AllPinsMask );
#endif

} // HBWLcSw16Hw


HmwDeviceHw::BasicConfig* HBWLcBl8Hw::getBasicConfig()
{
   return &config.basicConfig;
}