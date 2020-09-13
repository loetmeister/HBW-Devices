/*
 * Flash.h
 *
 *  Created on: 18.07.2017
 *      Author: Viktor Pankraz
 */

#ifndef Peripherals_Flash_H
#define Peripherals_Flash_H

#include <SwFramework.h>
#include <PersistentMemory.h>

class Flash : public PersistentMemory
{
   public:

      typedef uint32_t address_t;

      static uint8_t* setup( size_t _size );

      static void erase();

      static uint8_t read( uintptr_t offset );

      static uint16_t read( uintptr_t offset, void* pData, uint16_t length );

      static bool write( uintptr_t offset, uint8_t value );

      static uint16_t write( uintptr_t offset, const void* pData, uint16_t length );

   protected:

   private:

      inline Flash( size_t _size ) : PersistentMemory( "Flash", _size )
      {
         theFlash = this;
      }

      ////    Attributes    ////

   protected:

      static Flash* theFlash;
};

#endif

