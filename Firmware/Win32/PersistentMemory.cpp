/*
 * PersistentMemory.cpp
 *
 *  Created on: 18.07.2017
 *      Author: Viktor Pankraz
 */

#include "PersistentMemory.h"
#include <Tracing/Logger.h>
#include <string.h>

using namespace std;

const uint8_t PersistentMemory::debugLevel( DEBUG_LEVEL_OFF );

PersistentMemory::PersistentMemory( const char* name, size_t _size ) : fileName( name )
{
   DEBUG_H2( " setup for size: 0x", _size );
   BaseAddress = new uint8_t[_size];
   if ( BaseAddress != NULL )
   {
      size = _size;
      fstream theFile( name, ios::in );
      if ( theFile.good() )
      {
         DEBUG_H1( " read into local buffer" );
         theFile.read( (char*)BaseAddress, size );
      }
      else
      {
         DEBUG_M1( " not exists, start with empty memory" );
         memset( BaseAddress, 0xFF, _size );
         theFile.open( name, ios::out );
         theFile.write( (const char*)BaseAddress, _size );
      }
      theFile.close();
      return;
   }
   else
   {
      ERROR_2( getId(), " failed to allocate memory for buffer" );
   }
   size = 0;
}

void PersistentMemory::erase()
{
   DEBUG_H1( " erase" );
   memset( BaseAddress, 0xFF, size );
   commit();
}


uint8_t PersistentMemory::read( uintptr_t offset )
{
   // adjust memory mapped access
   if ( offset >= (uintptr_t)BaseAddress )
   {
      offset -= (uintptr_t)BaseAddress;
   }
   DEBUG_H2( " read at 0x", offset );
   if ( !isAddressInRange( offset ) )
   {
      return false;
   }
   DEBUG_M2( ' ', BaseAddress[offset] );
   return BaseAddress[offset];
}

uint16_t PersistentMemory::read( uintptr_t offset, void* pData, uint16_t length )
{
   // adjust memory mapped access
   if ( offset >= (uintptr_t)BaseAddress )
   {
      offset -= (uintptr_t)BaseAddress;
   }
   DEBUG_H4( " read at 0x", offset, ' ', length );
   if ( !isAddressInRange( offset ) || !isAddressInRange( offset + length ) )
   {
      return 0;
   }
   memcpy( pData, &BaseAddress[offset], length );
   DEBUG_M1( ' ' );
   for ( uint16_t i = 0; i < length; i++ )
   {
      DEBUG_L2( BaseAddress[offset + i], ' ' );
   }
   return length;
}

bool PersistentMemory::write( uintptr_t offset, uint8_t value )
{
   // adjust memory mapped access
   if ( offset >= (uintptr_t)BaseAddress )
   {
      offset -= (uintptr_t)BaseAddress;
   }
   DEBUG_H2( " write at 0x", offset );

   if ( !isAddressInRange( offset ) )
   {
      return false;
   }

   if ( BaseAddress[offset] != value )
   {
      BaseAddress[offset] = value;
      commit();
      DEBUG_M2( ' ', BaseAddress[offset] );
   }
   return true;
}

uint16_t PersistentMemory::write( uintptr_t offset, const void* pData, uint16_t length )
{
   // adjust memory mapped access
   if ( offset >= (uintptr_t)BaseAddress )
   {
      offset -= (uintptr_t)BaseAddress;
   }
   DEBUG_H4( " write at 0x", offset, ' ', length );
   if ( !isAddressInRange( offset ) || !isAddressInRange( offset + length ) )
   {
      return false;
   }

   if ( memcmp( &BaseAddress[offset], pData, length ) )
   {
      memcpy( &BaseAddress[offset], pData, length );
      commit();
      DEBUG_M1( ' ' );
      for ( uint16_t i = 0; i < length; i++ )
      {
         DEBUG_L2( BaseAddress[offset + i], ' ' );
      }
   }
   return length;
}

bool PersistentMemory::isAddressInRange( uintptr_t address )
{
   if ( address > size )
   {
      ERROR_3( getId(), " isOutOfRange 0x", address );
      return false;
   }
   return true;
}

bool PersistentMemory::commit()
{
   DEBUG_H1( " save data" );
   fstream theFile( fileName, ios::out );
   if ( theFile.good() )
   {
      theFile.write( (const char*)BaseAddress, size );
      size_t writtenBytes = theFile.tellg();
      theFile.close();
      if ( writtenBytes != size )
      {
         ERROR_4( "Could only write 0x", writtenBytes, " / 0x", size );
         return false;
      }
      return true;
   }
   ERROR_2( getId(), " could not be opened! Data not saved" );
   return false;
}
