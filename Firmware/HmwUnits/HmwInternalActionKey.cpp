#include "HmwInternalActionKey.h"

// Class HmwKey
HmwInternalActionKey::HmwInternalActionKey( PortPin _pin, HmwChannel* _actionChannel ) :
   actionChannel( _actionChannel ),
   digitalIn( _pin ),
   pressedCount( 0 )
{
   type = HmwChannel::HMW_KEY;
   digitalIn.enablePullup();
   enable( CHECK_INTERVALL );
}

void HmwInternalActionKey::loop()
{
   if ( actionChannel )
   {
      if ( digitalIn.isSet() )
      {
         pressedCount = 0;
      }
      else
      {
         if ( pressedCount < MAX_DEBOUNCE_COUNT )
         {
            if ( ++pressedCount == MAX_DEBOUNCE_COUNT )
            {
               uint8_t data[16];
               actionChannel->get( data );
               // toggle channel
               *data = ( *data == 0 ) ? MAX_LEVEL : 0;
               actionChannel->set( 1, data );
            }
         }
      }
      enable( CHECK_INTERVALL );
   }
   else
   {
      disable();
   }
}