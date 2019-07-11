/*
 * HmwLinkSender.h
 *
 * Created: 25.05.2019
 * Author: loetmeister.de, viktor.pankraz
 */


#ifndef __HMWLINKSENDERINFOEVENT_H__
#define __HMWLINKSENDERINFOEVENT_H__

#include <IStream.h>

class HmwLinkSenderInfoEvent
{
   protected:

      inline HmwLinkSenderInfoEvent()
      {
         instance = this;
      }

   private:

      static HmwLinkSenderInfoEvent* instance;

      // functions
   public:
      virtual IStream::Status sendInfoEvent( uint8_t srcChan, uint8_t const* const data, uint8_t length ) = 0;

      static inline IStream::Status notifyInfoEvent( uint8_t srcChan, uint8_t const* const data, uint8_t length )
      {
         if ( instance )
         {
            return instance->sendInfoEvent( srcChan, data, length );
         }
         return IStream::NOT_SUPPORTED;
      }
}; // HmwLinkSender

#endif // __HMWLINKSENDERINFOEVENT_H__
