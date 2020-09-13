/*
** HBWLinkDimmer
**
** Direkte Verknuepfung (Peering), zu Dimmern
** Ein Link-Objekt steht immer fuer alle (direkt aufeinander folgenden) Verknuepfungen
** des entsprechenden Typs.
**
*/

#include "HmwLinkBlindActor.h"
#include "HmwChannel.h"
#include "HmwDevice.h"


HmwLinkBlindActor::HmwLinkBlindActor( uint8_t _numLinks, Config* _links )
{
   numLinks = _numLinks;
   links = _links;
}

// processKeyEvent is called for each KEY event

void HmwLinkBlindActor::receiveKeyEvent( const uint32_t&  senderAddress, uint8_t senderChannel, uint8_t targetChannel, bool longPress, uint8_t keyNum )
{
   // read what to do from EEPROM
   for ( uint8_t i = 0; i < numLinks; i++ )
   {
      // compare sender channel
      if ( links[i].sensorChannel != senderChannel )
      {
         continue;
      }

      // compare target channel, ignore to match targetChannel if longPress is active
      if ( ( links[i].ownChannel != targetChannel ) && !longPress )
      {
         continue;
      }

      if ( links[i].sensorAddress == 0xFFFFFFFF )
      {
         continue;
      }

      // the endianess in the EEPROM is BigEndian, we need it in LittleEndian
      if ( changeEndianness( links[i].sensorAddress ) != senderAddress )
      {
         continue;
      }

      HmwBlindActor::LinkCommand cmd;
      cmd.keyNum = keyNum;
      cmd.actionParameter = ( longPress ? &links[i].longActionParameter : &links[i].shortActionParameter );

      HmwDevice::set( links[i].ownChannel, sizeof( cmd ), (uint8_t*)&cmd );    // channel, data length, data
   }
}


