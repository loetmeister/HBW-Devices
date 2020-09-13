/*
 * HBWMultiKeySD6BaseHw.h
 *
 * Created: 09.01.2018 18:36:42
 * Author: viktor.pankraz
 */


#ifndef __HBWLCSW8HW_H__
#define __HBWLCSW8HW_H__

#include "HBWLcBaseHw.h"

#include <HmwUnits/HmwSwitch.h>
#include <HmwUnits/HmwKey.h>
#include <HmwUnits/HmwInternalActionKey.h>
#include <HmwUnits/HmwLinkSwitch.h>
#include <HmwUnits/HmwLinkKey.h>
#include <PortPin.h>


class HBWLcSw8Hw : public HBWLcBaseHw
{
// variables
   public:

   protected:

      HmwSwitch hbwSwitch1, hbwSwitch2, hbwSwitch3, hbwSwitch4, hbwSwitch5, hbwSwitch6, hbwSwitch7, hbwSwitch8;
      HmwKey hbwKey1, hbwKey2, hbwKey3, hbwKey4, hbwKey5, hbwKey6, hbwKey7, hbwKey8;
      HmwInternalActionKey iKey1, iKey2, iKey3, iKey4, iKey5, iKey6, iKey7, iKey8;

   private:

// functions
   public:

      HBWLcSw8Hw();

   protected:

      HmwDeviceHw::BasicConfig* getBasicConfig();

   private:

      HmwLinkSwitch linkReceiver;
      HmwLinkKey linkSender;

}; // HBWLcSw8Hw

#endif // __HBWLCSW8HW_H__
