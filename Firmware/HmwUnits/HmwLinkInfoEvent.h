/*
** HBWLinkInfoEvent
**
** Einfache direkte Verknuepfung (Peering), vom Tastereingang ausgehend
** Ein Link-Objekt steht immer fuer alle (direkt aufeinander folgenden) Verknuepfungen
** des entsprechenden Typs.
**
*/


#ifndef HmwLinkInfoEvent_h
#define HmwLinkInfoEvent_h

#include "HmwLinkSenderInfoEvent.h"

class HmwLinkInfoEvent : public HmwLinkSenderInfoEvent
{
   public:

      struct Config
      {
         uint8_t ownChannel;
         uint32_t actorAddress;
         uint8_t actorChannel;
      };

      HmwLinkInfoEvent( uint8_t _numLinks, Config* _links );
      IStream::Status sendInfoEvent( uint8_t srcChan, uint8_t const* const data, uint8_t length );


   private:
      uint8_t numLinks;         // number of links of this type
      Config* links;            // size sollte konstant sein -> als define in .cpp
};

#endif
