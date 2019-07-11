/*
** HmwLinkInfoEvent
**
** Einfache direkte Verknuepfung (Peering), vom Tastereingang ausgehend
** Ein Link-Objekt steht immer fuer alle (direkt aufeinander folgenden) Verknuepfungen
** des entsprechenden Typs.
**
*/


#include "HmwLinkInfoEvent.h"
#include "HmwChannel.h"
#include "HmwDevice.h"


HmwLinkInfoEvent::HmwLinkInfoEvent( uint8_t _numLinks, Config* _links )
{
   numLinks = _numLinks;
   links = _links;
}


// sendInfoEvent wird aufgerufen, wenn neue daten bereit stehen
IStream::Status HmwLinkInfoEvent::sendInfoEvent( uint8_t srcChan, uint8_t const* const data, uint8_t length )
{
   IStream::Status status = IStream::NO_DATA;

   // care for peerings
   for ( int i = 0; i < numLinks; i++ )
   {
      // valid peering?
      if ( links[i].ownChannel == 0xFF )
      {
         continue;
      }
      // channel is key?
      if ( links[i].ownChannel != srcChan )
      {
         continue;
      }

      // at least one peering fired
      status = IStream::SUCCESS;
      uint32_t actorAddress = changeEndianness( links[i].actorAddress );

      // own address? -> internal peering
      if ( actorAddress == HmwDevice::ownAddress )
      {
		  // internal peering not supported
         //HmwDevice::receiveInfoEvent( actorAddress, srcChan, links[i].actorChannel, longPress, keyPressNum );
      }
      else
      {
         // external peering
         HmwDevice::sendInfoEvent( srcChan, data, length, actorAddress, links[i].actorChannel );
      }
   }

   return status;
}


