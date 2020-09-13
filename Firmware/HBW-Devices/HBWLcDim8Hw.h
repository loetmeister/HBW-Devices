/*
 * HBWMultiKeySD6BaseHw.h
 *
 * Created: 09.01.2018 18:36:42
 * Author: viktor.pankraz
 */


#ifndef __HBWLCDIM8HW_H__
#define __HBWLCDIM8HW_H__

#include "HBWLcBaseHw.h"

#include <HmwUnits/HmwDimmer.h>
#include <HmwUnits/HmwKey.h>
#include <HmwUnits/HmwLinkDimmer.h>
#include <HmwUnits/HmwLinkKey.h>
#include <HmwUnits/HmwDS1820.h>


class HBWLcDim8Hw : public HBWLcBaseHw
{
// variables
   public:

   protected:

      HmwDimmer hbwDimmer1, hbwDimmer2, hbwDimmer3, hbwDimmer4, hbwDimmer5, hbwDimmer6, hbwDimmer7, hbwDimmer8;
      HmwKey hbwKey1, hbwKey2, hbwKey3, hbwKey4, hbwKey5, hbwKey6, hbwKey7, hbwKey8, hbwKey9, hbwKey10;
      OneWire ow;
      HmwDS1820 hbwTmp1, hbwTmp2, hbwTmp3, hbwTmp4, hbwTmp5, hbwTmp6;

   private:

      static const uint16_t ZCD_DEFAULT_PERIOD = 41000;

      HmwLinkDimmer linkReceiver;
      HmwLinkKey linkSender;

// functions
   public:

      HBWLcDim8Hw();

   protected:

      HmwDeviceHw::BasicConfig* getBasicConfig();

   private:

      void configureZeroCrossDetection();

}; // HBWLcDim8Hw

#endif // __HBWLCDIM8HW_H__
