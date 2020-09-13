/*
 * Usart.h
 *
 *  Created on: 18.07.2017
 *      Author: Viktor Pankraz
 */

#ifndef Peripherals_Usart_H
#define Peripherals_Usart_H

#include <CriticalSection.h>
#include <SwFramework.h>

/* Character Size */
typedef enum USART_CHSIZE_enum
{
   USART_CHSIZE_5BIT_gc = ( 0x00 << 0 ), /* Character size: 5 bit */
   USART_CHSIZE_6BIT_gc = ( 0x01 << 0 ), /* Character size: 6 bit */
   USART_CHSIZE_7BIT_gc = ( 0x02 << 0 ), /* Character size: 7 bit */
   USART_CHSIZE_8BIT_gc = ( 0x03 << 0 ), /* Character size: 8 bit */
   USART_CHSIZE_9BIT_gc = ( 0x07 << 0 ), /* Character size: 9 bit */
} USART_CHSIZE_t;

/* Communication Mode */
typedef enum USART_CMODE_enum
{
   USART_CMODE_ASYNCHRONOUS_gc = ( 0x00 << 6 ), /* Asynchronous Mode */
   USART_CMODE_SYNCHRONOUS_gc = ( 0x01 << 6 ), /* Synchronous Mode */
   USART_CMODE_IRDA_gc = ( 0x02 << 6 ), /* IrDA Mode */
   USART_CMODE_MSPI_gc = ( 0x03 << 6 ), /* Master SPI Mode */
} USART_CMODE_t;

/* Parity Mode */
typedef enum USART_PMODE_enum
{
   USART_PMODE_DISABLED_gc = ( 0x00 << 4 ), /* No Parity */
   USART_PMODE_EVEN_gc = ( 0x02 << 4 ), /* Even Parity */
   USART_PMODE_ODD_gc = ( 0x03 << 4 ), /* Odd Parity */
} USART_PMODE_t;

class Usart
{
   public:

      template<uint32_t baudrate, USART_CMODE_t mode = USART_CMODE_ASYNCHRONOUS_gc,
               USART_PMODE_t parity = USART_PMODE_DISABLED_gc, USART_CHSIZE_t characterSize = USART_CHSIZE_8BIT_gc,
               bool twoStopBits = false, bool doubleClock = true>
      inline bool init()
      {
         return false; // no error
      }

      inline void enableReceiveCompleteInterrupt()
      {
      }

      inline void waitUntilTransferCompleted()
      {
      }

      inline bool isReceiveCompleted()
      {
         return false;
      }

      inline bool read( uint8_t& data )
      {
         return false;
      }

      inline bool write( uint8_t data )
      {
         return true;
      }

   protected:

   private:

      inline Usart()
      {
      }

      ////    Attributes    ////

   protected:

      static Usart theUsart;
};

#endif

