/*
 * NetworkAdapter.cpp
 *
 *  Created on: 18.04.2019
 *      Author: Viktor Pankraz
 */

#include "NetworkAdapter.h"

#include <Protocols\Ethernet\EthernetHeader.h>
#include <Protocols\Ethernet\IP.h>
#include <Protocols\Ethernet\IpHeader.h>
#include <Protocols\Ethernet\UdpHeader.h>

#include <iostream>

// some defines from windows.h must be copied here because including windows.h causes conflict with IStream
#define AF_INET                                    2

const char NetworkAdapter::defaultFilter[] = "arp or udp dst port 9 or 67 or 68 or icmp or tcp port 501 or 502";

const char NetworkAdapter::udpPort9Filter[] = "udp dst port 9";

const uint8_t NetworkAdapter::debugLevel( DEBUG_LEVEL_HIGH );

FlashString* NetworkAdapter::getId()
{
   return FSTR( "NetworkAdapter::" );
}

NetworkAdapter::NetworkAdapter() : available( true ), deviceHandle( NULL )
{
}

NetworkAdapter::~NetworkAdapter()
{
   if ( deviceHandle != NULL )
   {
      pcap_close( deviceHandle );
   }
}

bool NetworkAdapter::configure()
{
   pcap_if_t* alldevs;
   pcap_if_t* d;
   pcap_if_t* validDevice = 0;
   char errbuf[PCAP_ERRBUF_SIZE + 1];

   if ( pcap_findalldevs( &alldevs, errbuf ) == -1 )
   {
      ERROR_2( "Error in pcap_findalldevs: ", errbuf );
      return false;
   }

   for ( d = alldevs; d; d = d->next )
   {
      dumpDevice( d );
      if ( getIp( d ).isValid() )
      {
         if ( !validDevice )
         {
            validDevice = d;
            DEBUG_M1( "selected" );
         }
         else
         {
            Logger::instance() << "There is more than one device valid. Please decide which one to use." << endl;
            Logger::instance() << "- currently selected: " << validDevice->description << endl;
            Logger::instance() << "- new available     : " << d->description << endl;
            Logger::instance() << "- select the new one? (y/n): ";
            char decision;
            std::cin >> decision;
            Logger::instance() << endl;
            if ( decision == 'y' )
            {
               validDevice = d;
               Logger::instance() << " - new interface selected! " << endl;
            }
         }

      }
   }
   if ( !validDevice )
   {
      ERROR_1( "no valid device found" );
      return false;
   }

   // Open the adapter
   deviceHandle = pcap_open_live( validDevice->name,    // name of the device
                                  BUFFER_SIZE,          // portion of the packet to capture.
                                  1,                    // promiscuous mode (nonzero means promiscuous)
                                  1,                    // read timeout
                                  errbuf                // error buffer
                                  );
   if ( deviceHandle == NULL )
   {
      ERROR_3( "Unable to open the adapter.", validDevice->name, " is not supported by WinPcap" );
      // Free the device list
      pcap_freealldevs( alldevs );
      return false;
   }

   DEBUG_H2( "listening on ", validDevice->description );

   // At this point, we don't need any more the device list. Free it
   pcap_freealldevs( alldevs );
   setFilter( defaultFilter );

   return true;
}

IStream::Status NetworkAdapter::genericCommand( IoStream::Command command, void* buffer )
{
   if ( command == IoStream::IS_LINKED_UP )
   {
      CommandIS_LINKED_UP* data = (CommandIS_LINKED_UP*)buffer;
      data->linkedUp = available;
   }
   else if ( command == IoStream::SET_FILTER )
   {
      CommandSET_FILTER* data = (CommandSET_FILTER*)buffer;
      setFilter( data->filter );
   }
   else
   {
      return IStream::NOT_SUPPORTED;
   }
   return IStream::SUCCESS;
}

IStream::Status NetworkAdapter::init()
{
   if ( available )
   {
      // available &= connect();
      available &= configure();
   }
   return available ? IStream::SUCCESS : IStream::NOT_SUPPORTED;
}

IStream::Status NetworkAdapter::read( void* pData, uint16_t length, EventDrivenUnit* user )
{
   struct pcap_pkthdr* header;
   const u_char* pkt_data;

   int res = pcap_next_ex( deviceHandle, &header, &pkt_data );

   if ( res < 0 )
   {
      ERROR_2( "Error reading the packets: ", pcap_geterr( deviceHandle ) );
   }
   if ( res == 1 )
   {
      if ( !( (const EthernetHeader*)pkt_data )->getPacketSource()->isLocal() )
      {
         DEBUG_M2( FSTR( "received bytes: 0x" ), header->len );
         if ( header->len > length )
         {
            ERROR_1( "Error reading the packets: too many bytes received" );
         }
         else
         {
            memcpy( pData, pkt_data, header->len );
            return IStream::SUCCESS;
         }
      }
   }
   return IStream::NO_DATA;
}

void NetworkAdapter::setFilter( const char* filter )
{
   // We should loop through the adapters returned by the pcap_findalldevs_ex()
   // in order to locate the correct one.
   //
   // Let's do things simpler: we suppose to be in a C class network ;-)
   uint32_t NetMask = 0xffffff;
   struct bpf_program fcode;

   // compile the filter
   if ( pcap_compile( deviceHandle, &fcode, filter, 1, NetMask ) < 0 )
   {
      ERROR_2( "Error compiling filter: ", filter );
   }

   // set the filter
   if ( pcap_setfilter( deviceHandle, &fcode ) < 0 )
   {
      ERROR_1( "Error setting the filter" );
   }
}

IStream::Status NetworkAdapter::write( void* pData, uint16_t length, EventDrivenUnit* user )
{
   DEBUG_H3( "sending 0x", length, " bytes" );

   if ( deviceHandle )
   {
      if ( pcap_sendpacket( deviceHandle, (const uint8_t*)pData, length ) == 0 )
      {
         return IStream::SUCCESS;
      }
      ERROR_2( "Error sending the packet: ", pcap_geterr( deviceHandle ) );
      return IStream::ABORTED;
   }
   return IStream::NOT_SUPPORTED;
}

void NetworkAdapter::dumpDevice( pcap_if* d )
{
   Logger::instance() << d->name << endl;
   for ( pcap_addr_t* a = d->addresses; a != NULL; a = a->next )
   {
      if ( a->addr->sa_family == AF_INET )
      {
         IP ip{ .address = (uint32_t)( (struct sockaddr_in*)a->addr )->sin_addr.s_addr };
         Logger::instance() << "-IP: " << ip << endl;
      }
   }
}

IP NetworkAdapter::getIp( pcap_if* d )
{
   for ( pcap_addr_t* a = d->addresses; a != NULL; a = a->next )
   {
      if ( a->addr->sa_family == AF_INET )
      {
         IP ip{ .address = (uint32_t)( (struct sockaddr_in*)a->addr )->sin_addr.s_addr };
         return ip;
      }
   }
   return IP();
}
