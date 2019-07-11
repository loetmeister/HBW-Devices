/*
 * HmwMsgInfoEvent.h
 *
 * Created: 25.05.2019
 * Author: loetmeister.de, viktor.pankraz, Thorsten Pferdekaemper thorsten@pferdekaemper.com
 *      Nach einer Vorlage von Dirk Hoffmann (dirk@hfma.de) 2012
 */


#ifndef __HMWMSGINFOEVENT_H__
#define __HMWMSGINFOEVENT_H__

#include "HmwMessageBase.h"

class HmwMsgInfoEvent : public HmwMessageBase
{
// variables
   public:
   protected:
   private:

// functions
   public:
      inline HmwMsgInfoEvent( uint32_t ownAddress, uint32_t dstAddress, uint8_t srcChannel, uint8_t dstChannel, uint8_t const* const data, uint8_t length )
      {
         frameData[0] = INFO_EVENT;//INFO_LEVEL;//
         frameData[1] = srcChannel;
         frameData[2] = dstChannel;
         memcpy( &frameData[3], data, length );
         controlByte = 0xF8;
         senderAddress = ownAddress;
         targetAddress = dstAddress;
         frameDataLength = 0x03 + length;
      }

      inline uint8_t getSourceChannel()
      {
         return frameData[1];
      }

      inline uint8_t getDestinationChannel()
      {
         return frameData[2];
      }

   protected:
   private:


}; // HmwMsgInfoEvent

#endif // __HMWMSGINFOEVENT_H__
