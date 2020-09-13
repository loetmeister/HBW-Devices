#ifndef HmwInternalActionKey_h
#define HmwInternalActionKey_h

#include "HmwChannel.h"
#include <DigitalInput.h>
#include <Time/Timestamp.h>
#include <xEeprom.h>


// Class HBWKey
class HmwInternalActionKey : public HmwChannel
{
   public:

      static const uint8_t CHECK_INTERVALL = 10;
      static const uint8_t MAX_DEBOUNCE_COUNT = 4;

      HmwInternalActionKey( PortPin _pin, HmwChannel* _actionChannel = NULL );

      inline void setActionChannel( HmwChannel* _actionChannel )
      {
         actionChannel = _actionChannel;
         enable( CHECK_INTERVALL );
      }

      inline const DigitalInput& getDigitalInput() const
      {
         return digitalIn;
      }

      virtual void loop();

   protected:

   private:

      HmwChannel* actionChannel;

      DigitalInput digitalIn;

      uint8_t pressedCount;
};

#endif