/*
 * WatchDog.h
 *
 *  Created on: 17.07.2017
 *      Author: Viktor Pankraz
 */

#ifndef Peripherals_WatchDog_H
#define Peripherals_WatchDog_H

class WatchDog
{
   public:

      enum Timeout {
         _8MS,
         _16MS,
         _32MS,
         _64MS,
         _125MS,
         _250MS,
         _500MS,
         _1S,
         _2S,
         _4S,
         _8S,
      };

      ////    Operations    ////

      ///*! \brief This function lock the entire clock system configuration.
      // *
      // *  This will lock the configuration until the next reset, or until the
      // *  External Oscillator Failure Detections (XOSCFD) feature detects a failure
      // *  and switches to internal 2MHz RC oscillator.
      // */
      inline static void disable()
      {

      }

      ///*! \brief This function lock the entire clock system configuration.
      // *
      // *  This will lock the configuration until the next reset, or until the
      // *  External Oscillator Failure Detections (XOSCFD) feature detects a failure
      // *  and switches to internal 2MHz RC oscillator.
      // */
      inline static void enable( Timeout timeout )
      {

      }

      ///*! \brief This function lock the entire clock system configuration.
      // *
      // *  This will lock the configuration until the next reset, or until the
      // *  External Oscillator Failure Detections (XOSCFD) feature detects a failure
      // *  and switches to internal 2MHz RC oscillator.
      // */
      inline static void reset()
      {

      }

   private:

};

#endif
