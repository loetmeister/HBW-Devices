/*
** HmwLinkSwitch
**
** Direkte Verknuepfung (Peering), zu Relays
** Ein Link-Objekt steht immer fuer alle (direkt aufeinander folgenden) Verknuepfungen
** des entsprechenden Typs.
**
*/

#ifndef HmwLinkSwitch_h
#define HmwLinkSwitch_h

#include "HmwLinkReceiver.h"
#include "HmwSwitch.h"

class HmwLinkSwitch : public HmwLinkReceiver
{
   public:

      struct Config  // sizeof = 28 Bytes
      {
         uint32_t sensorAddress;
         uint8_t sensorChannel;
         uint8_t ownChannel;
         HmwSwitch::ActionParameter shortActionParameter;
         HmwSwitch::ActionParameter longActionParameter;
      };

      HmwLinkSwitch( uint8_t _numLinks, Config* _links );
      void receiveKeyEvent( const uint32_t&  senderAddress, uint8_t senderChannel, uint8_t targetChannel, bool longPress, uint8_t keyNum );

   private:
      uint8_t numLinks;         // number of links of this type
      Config* links;
};

#endif
