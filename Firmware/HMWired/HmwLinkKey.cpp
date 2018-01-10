/*
** HmwLinkKey
**
** Einfache direkte Verknuepfung (Peering), vom Tastereingang ausgehend
** Ein Link-Objekt steht immer fuer alle (direkt aufeinander folgenden) Verknuepfungen
** des entsprechenden Typs.
**
*/


#include "HmwLinkKey.h"
#include "HmwChannel.h"
#include "HmwDevice.h"


HmwLinkKey::HmwLinkKey( uint8_t _numLinks, Config* _links )
{
   numLinks = _numLinks;
   links = _links;
}


// keyPressed wird aufgerufen, wenn ein Tastendruck erkannt wurde
Stream::Status HmwLinkKey::sendKeyEvent( uint8_t srcChan, uint8_t keyPressNum, bool longPress )
{
   Stream::Status status = Stream::NO_DATA;

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
      status = Stream::SUCCESS;
      uint32_t actorAddress = changeEndianness( links[i].actorAddress );

      // own address? -> internal peering
      if ( actorAddress == HmwDevice::ownAddress )
      {
         HmwDevice::receiveKeyEvent( actorAddress, srcChan, links[i].actorChannel, longPress );
      }
      else
      {
         // external peering
         HmwDevice::sendKeyEvent( srcChan, keyPressNum, longPress, actorAddress, links[i].actorChannel );
      }
   }

   return status;
}

