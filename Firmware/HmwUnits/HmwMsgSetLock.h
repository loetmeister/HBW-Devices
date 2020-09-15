/*
 * HmwMsgSetLock.h
 *
 * Created: 15.09.2020
 * Author:
 */


#ifndef __HMWMSGSETLOCK_H__
#define __HMWMSGSETLOCK_H__

#include "HmwMessageBase.h"

class HmwMsgSetLock : public HmwMessageBase
{
// variables
   public:
   protected:
   private:

// functions
   public:
      inline uint8_t getChannel()
      {
         return frameData[2];
      }

      inline bool getData()
      {
         return (frameData[3] & 0x01);
      }

   protected:
   private:

}; // HmwMsgSetLock

#endif // __HMWMSGSETLOCK_H__
