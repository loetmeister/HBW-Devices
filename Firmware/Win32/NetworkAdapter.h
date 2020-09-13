/*
 * NetworkAdapter.h
 *
 *  Created on: 18.04.2019
 *      Author: Viktor Pankraz
 */

#ifndef Simulators_PcapPkg_NetworkAdapter_H
#define Simulators_PcapPkg_NetworkAdapter_H

#include <pcap/pcap.h>
#include <IoStream.h>

class FlashString;

class IP;

class NetworkAdapter : public IoStream
{
   public:

      static const uint16_t BUFFER_SIZE = 2048;

      ////    Constructors and destructors    ////

      NetworkAdapter();

      ~NetworkAdapter();

      ////    Operations    ////

      bool configure();

      static FlashString* getId();

      IStream::Status  init();


      virtual IStream::Status genericCommand( Command command, void* buffer );

      virtual IStream::Status read( void* pData, uint16_t length, EventDrivenUnit* user = 0 );

      virtual IStream::Status write( void* pData, uint16_t length, EventDrivenUnit* user = 0 );

      void setFilter( const char* filter );

   private:

      void dumpDevice( pcap_if* d );

      IP getIp( pcap_if* d );

      ////    Additional operations    ////

   public:


   protected:

      ////    Attributes    ////

   public:

      bool available; // ## attribute available

      static const char defaultFilter[];  // ## attribute defaultFilter

      pcap* deviceHandle; // ## attribute deviceHandle

      static const char udpPort9Filter[];   // ## attribute udpPort9Filter

   protected:

      static const uint8_t debugLevel;  // ## attribute debugLevel

};


#endif
