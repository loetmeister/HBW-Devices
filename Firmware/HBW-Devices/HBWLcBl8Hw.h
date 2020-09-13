/*
 * HBWLcBl8Hw.h
 *
 * Created: 09.01.2019 18:36:42
 * Author: viktor.pankraz
 */


#ifndef __HBWLCBL8HW_H__
#define __HBWLCBL8HW_H__

#include "HBWLcBaseHw.h"

#include <HmwUnits/HmwBlindActor.h>
#include <HmwUnits/HmwKey.h>
#include <HmwUnits/HmwLinkBlindActor.h>
#include <HmwUnits/HmwLinkKey.h>
#include <HmwUnits/HmwDS1820.h>
#include <PortPin.h>


class HBWLcBl8Hw : public HBWLcBaseHw
{
// variables
   public:

   protected:

      HmwBlindActor hbwBlindActor1, hbwBlindActor2, hbwBlindActor3, hbwBlindActor4, hbwBlindActor5, hbwBlindActor6, hbwBlindActor7, hbwBlindActor8;
      HmwKey hbwKey1, hbwKey2, hbwKey3, hbwKey4, hbwKey5, hbwKey6, hbwKey7, hbwKey8, hbwKey9, hbwKey10;
      OneWire ow;
      HmwDS1820 hbwTmp1, hbwTmp2, hbwTmp3, hbwTmp4, hbwTmp5, hbwTmp6;

   private:

// functions
   public:

      HBWLcBl8Hw();

   protected:

      HmwDeviceHw::BasicConfig* getBasicConfig();

   private:

      HmwLinkBlindActor linkReceiver;
      HmwLinkKey linkSender;

}; // HBWLcSw16Hw

#endif // __HBWLCBL8HW_H__
