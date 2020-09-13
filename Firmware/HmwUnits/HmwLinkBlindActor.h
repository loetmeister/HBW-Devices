/*
** HmwLinkBlindActor.h
**
** Direkte Verknuepfung (Peering), zu Rollladen
** Ein Link-Objekt steht immer fuer alle (direkt aufeinander folgenden) Verknuepfungen
** des entsprechenden Typs.
**
*/

#ifndef HmwLinkBlindActor_h
#define HmwLinkBlindActor_h

#include "HmwLinkReceiver.h"
#include "HmwBlindActor.h"

class HmwLinkBlindActor : public HmwLinkReceiver
{
   public:

      struct Config  // sizeof = 38Bytes
      {
         uint32_t sensorAddress;
         uint8_t sensorChannel;
         uint8_t ownChannel;
         HmwBlindActor::ActionParameter shortActionParameter;
         HmwBlindActor::ActionParameter longActionParameter;
      };

      HmwLinkBlindActor( uint8_t _numLinks, Config* _links );
      void receiveKeyEvent( const uint32_t&  senderAddress, uint8_t senderChannel, uint8_t targetChannel, bool longPress, uint8_t keyNum );

   private:
      uint8_t numLinks;         // number of links of this type
      Config* links;
};

#endif
