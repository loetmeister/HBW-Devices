/*
 * xEeprom.h
 *
 *  Created on: 18.07.2017
 *      Author: Viktor Pankraz
 */

#ifndef XEEPROM_H
#define XEEPROM_H

#include "Eeprom.h"


template<typename T>
class XEeprom
{
   private:
      // never call constructor the object itself points to eeprom cell
      inline XEeprom()
      {
      }
      T member;

   public:

      // Access/read members.
      T operator*() const
      {
         return member;
      }
      operator const T() const { return **this; }

      // Assignment/write members.
      XEeprom<T>& operator=( const XEeprom<T>& ref )
      {
         return *this = *ref;
      }

      XEeprom<T>& operator=( T in )
      {
         Eeprom::write( (uintptr_t)this, &in, sizeof( T ) );
         return *this;
      }

      XEeprom<T>& operator +=( T in )
      {
         return *this = **this + in;
      }
      XEeprom<T>& operator -=( T in )
      {
         return *this = **this - in;
      }
      XEeprom<T>& operator *=( T in )
      {
         return *this = **this * in;
      }
      XEeprom<T>& operator /=( T in )
      {
         return *this = **this / in;
      }
      XEeprom<T>& operator ^=( T in )
      {
         return *this = **this ^ in;
      }
      XEeprom<T>& operator %=( T in )
      {
         return *this = **this % in;
      }
      XEeprom<T>& operator &=( T in )
      {
         return *this = **this & in;
      }
      XEeprom<T>& operator |=( T in )
      {
         return *this = **this | in;
      }
      XEeprom<T>& operator <<=( uint8_t in )
      {
         return *this = **this << in;
      }
      XEeprom<T>& operator >>=( uint8_t in )
      {
         return *this = **this >> in;
      }

      XEeprom<T>& update( T in )
      {
         return in != *this ? *this = in : *this;
      }


      // Prefix increment/decrement
      XEeprom& operator++()
      {
         return *this += 1;
      }
      XEeprom& operator--()
      {
         return *this -= 1;
      }


      // Postfix increment/decrement
      T operator++( int )
      {
         T ret = **this;
         return ++( *this ), ret;
      }

      T operator--( int )
      {
         T ret = **this;
         return --( *this ), ret;
      }
} __attribute__( ( packed ) );

typedef XEeprom<uint8_t>    uint8_tx;
typedef XEeprom<uint16_t>   uint16_tx;
typedef XEeprom<uint32_t>   uint32_tx;
typedef XEeprom<int8_t>     int8_tx;
typedef XEeprom<int16_t>    int16_tx;
typedef XEeprom<int32_t>    int32_tx;
typedef XEeprom<char>       char_tx;

#endif



