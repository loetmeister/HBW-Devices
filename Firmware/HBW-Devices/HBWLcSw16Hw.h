/*
 * HBWMultiKeySD6BaseHw.h
 *
 * Created: 09.01.2018 18:36:42
 * Author: viktor.pankraz
 */


#ifndef __HBWLCSW16HW_H__
#define __HBWLCSW16HW_H__

#include "HBWLcBaseHw.h"

#include <HmwUnits/HmwSwitch.h>
#include <HmwUnits/HmwKey.h>
#include <HmwUnits/HmwLinkSwitch.h>
#include <HmwUnits/HmwLinkKey.h>
#include <HmwUnits/HmwDS1820.h>
#include <PortPin.h>


class HBWLcSw16Hw : public HBWLcBaseHw
{
// variables
   public:

   protected:

      HmwSwitch hbwSwitch1, hbwSwitch2, hbwSwitch3, hbwSwitch4, hbwSwitch5, hbwSwitch6, hbwSwitch7, hbwSwitch8,
                           hbwSwitch9, hbwSwitch10, hbwSwitch11, hbwSwitch12, hbwSwitch13, hbwSwitch14, hbwSwitch15, hbwSwitch16;
      HmwKey hbwKey1, hbwKey2, hbwKey3, hbwKey4, hbwKey5, hbwKey6, hbwKey7, hbwKey8, hbwKey9, hbwKey10;
      OneWire ow;
      HmwDS1820 hbwTmp1, hbwTmp2, hbwTmp3, hbwTmp4, hbwTmp5, hbwTmp6;

   private:

// functions
   public:

      HBWLcSw16Hw();

   protected:

      HmwDeviceHw::BasicConfig* getBasicConfig();

   private:

      HmwLinkSwitch linkReceiver;
      HmwLinkKey linkSender;

}; // HBWLcSw16Hw

#endif // __HBWLCSW16HW_H__
