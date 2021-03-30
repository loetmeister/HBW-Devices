/*
 * HmwDevice.h
 *
 * Created: 06.10.2017 22:40:44
 * Author: viktor.pankraz
 */


#ifndef __HMWDEVICE_H__
#define __HMWDEVICE_H__

#include "HmwStream.h"
#include "HmwMsgAnnounce.h"
#include "HmwMsgKeyEvent.h"
#include "HmwMsgInfo.h"
#include "HmwMsgInfoEvent.h"
#include "HmwDeviceHw.h"

#include <DigitalOutput.h>

class HmwDevice
{
// variables
   public:


      struct PendingActions
      {
         uint8_t readConfig : 1;
         uint8_t announce : 1;
         uint8_t resetSystem : 1;
         uint8_t startBooter : 1;
         uint8_t startFirmware : 1;
         uint8_t resetWifiConnection : 1;
         uint8_t zeroCommunicationActive : 1;

      };

      static HmwDeviceHw::BasicConfig* basicConfig;

      static PendingActions pendingActions;

      static uint8_t deviceType;

      static uint32_t ownAddress;


   protected:

      static HmwDeviceHw* hardware;

   private:

      static const uint8_t debugLevel;

      static const uint32_t MIN_ADDRESS = 1620000000;

      static const uint32_t MAX_ADDRESS = 1629999999;

      static const uint32_t DEFAULT_ADDRESS = MIN_ADDRESS;

      static const SystemTime::time_t FIRST_ANNOUNCEMENT_TIME = 1500;


// functions
   public:

      static inline void setup( uint8_t _deviceType, HmwDeviceHw::BasicConfig* _config )
      {
         deviceType = _deviceType;
         basicConfig = _config;
         setOwnAddress( changeEndianness( basicConfig->ownAddress ) );
         pendingActions.announce = true;
      }

      static void setHardware( uint8_t _deviceType, HmwDeviceHw* _hardware )
      {
         if ( _hardware )
         {
            hardware = _hardware;
            HmwStream::setHardware( _hardware );
            pendingActions.readConfig = true;
            setup( _deviceType, _hardware->getBasicConfig() );
         }
      }

      static inline void setOwnAddress( uint32_t address )
      {
         if ( ( address >= MIN_ADDRESS ) && ( address <= MAX_ADDRESS ) )
         {
            ownAddress = address;
         }
      }

      static inline IStream::Status announce( uint8_t channel = 0 )
      {
         HmwMsgAnnounce msg( channel, ownAddress, deviceType, basicConfig->hwVersion, ( Release::MAJOR << 8 ) | Release::MINOR );
      #ifdef _BOOTER_
         return HmwStreamBase::sendMessage( msg );
      #else
         return HmwStream::sendMessage( msg );
      #endif
      }

      static inline bool isReadConfigPending()
      {
         return pendingActions.readConfig;
      }

      static inline void clearPendingReadConfig()
      {
         pendingActions.readConfig = false;
      }

      static inline bool isStartFirmwarePending()
      {
         return pendingActions.startFirmware;
      }

      static inline void clearPendingStartFirmware()
      {
         pendingActions.startFirmware = false;
      }

      static inline bool isAnnouncementPending()
      {
         return pendingActions.announce;
      }

      static inline void clearPendingAnnouncement()
      {
         pendingActions.announce = false;
      }

      static inline uint8_t getLoggingTime()
      {
         return basicConfig->loggingTime;
      }

      static void loop();

      static void checkConfig();

      static void factoryReset();
	  
	  static void setLock( uint8_t channel, bool inhibit );
	  
	  static bool getLock( uint8_t channel );

      static bool processMessage( HmwMessageBase& msg );

      static void handleAnnouncement();

      static uint8_t get( uint8_t channel, uint8_t* data );

      static void set( uint8_t channel, uint8_t length, uint8_t const* const data );

      static inline void receiveKeyEvent( const uint32_t& senderAddress, uint8_t srcChan, uint8_t dstChan, bool longPress, uint8_t keyPressNum )
      {
		  if ( !getLock( dstChan ) )   // check if channel is locked
          {
             HmwLinkReceiver::notifyKeyEvent( senderAddress, srcChan, dstChan, longPress, keyPressNum );
		  }
      }

      static inline IStream::Status sendKeyEvent( uint8_t srcChan, uint8_t keyPressNum, bool longPress, bool keyPressed = false )
      {
         IStream::Status status = sendKeyEvent( srcChan, keyPressNum, longPress, 0xFFFFFFFF, 0 );
         if ( status == IStream::SUCCESS )
         {
            HmwLinkSender::notifyKeyEvent( srcChan, keyPressNum, longPress );
            //if ( !keyPressed )
            if ( !keyPressed && keyPressNum == 0 )
            {
               pendingActions.announce = true;   // send announcement at first press/released key state (repeated every 64 key presses, when counter rolls over)
            }
         }
         return status;
      }

      static inline IStream::Status sendKeyEvent( uint8_t srcChan, uint8_t keyPressNum, bool longPress, uint32_t targetAddr, uint8_t targetChan )
      {
         HmwMsgKeyEvent msg( ownAddress, targetAddr, srcChan, targetChan, keyPressNum, longPress );
         return HmwStream::sendMessage( msg );
      }

      static inline IStream::Status sendInfoMessage( uint8_t channel, uint8_t length, uint8_t const* const data, uint32_t target_address = 0 )
      {
         HmwMsgInfo msg( ownAddress, target_address ? target_address : changeEndianness( basicConfig->centralAddress ), channel, data, length );
         return HmwStream::sendMessage( msg );
      }
#if defined(_Support_HBWLink_InfoEvent_)
       /* direkt aufeinanderfolgendes senden funktioniert nicht richtig. Eventuell da beide Nachrichten ein ACK erwarten? */
      /*static inline IStream::Status sendInfoMsgAndEvent( uint8_t srcChan, uint8_t const* const data, uint8_t length )-      {
         //return HmwLinkSenderInfoEvent::notifyInfoEvent( srcChan, data, length );
                uint8_t status;
                //status = sendInfoMessage( srcChan, length, data );
                status = 0;
                if ( status == 0 )
                {
                        length = 2;    // overwrite, to only send 2 bytes for temperature
                        HmwLinkSenderInfoEvent::notifyInfoEvent( srcChan, data, length );
                        return IStream::SUCCESS;
                }
                return IStream::ABORTED;
      }*/

      static inline IStream::Status sendInfoEvent( uint8_t srcChan, uint8_t const* const data, uint8_t length )
      {
         return HmwLinkSenderInfoEvent::notifyInfoEvent( srcChan, data, length );
      }

      static inline IStream::Status sendInfoEvent( uint8_t srcChan, uint8_t const* const data, uint8_t length, uint32_t targetAddr, uint8_t targetChan )
      {
         HmwMsgInfoEvent msg( ownAddress, targetAddr, srcChan, targetChan, data, length );
         return HmwStream::sendMessage( msg );
      }
#endif

   protected:

   private:

      static void handlePendingActions();

      static void handlePendingInMessages();

      static void handleConfigButton();

}; // HmwDevice

#endif // __HMWDEVICE_H__
