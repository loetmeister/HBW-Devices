/*
 * HmwMsgReset.h
 *
 * Created: 21.01.2021
 * Author: loetmeister
 */


#ifndef __HMWMSGRESET_H__
#define __HMWMSGRESET_H__

#include "HmwMessageBase.h"


class HmwMsgReset : public HmwMessageBase
{
// variables
   public:
   protected:
   private:

// functions
   public:
      inline bool isReset()
      {
         return (frameData[1] == RESET);
      }

   protected:
   private:

}; // HmwMsgReset

#endif // __HMWMSGRESET_H__
