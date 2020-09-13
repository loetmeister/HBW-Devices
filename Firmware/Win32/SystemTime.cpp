/*
 * SystemTime.cpp
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#include <Time/SystemTime.h>
int8_t SystemTime::ticksPerSecondAdjustment( 0 );
#include <windows.h>
#include <ctime>

SystemTime::time_t SystemTime::now()
{
   return std::clock();
}

SystemTime::time_t SystemTime::since( const SystemTime::time_t& lastTime )
{
   time_t elapsedTime, timestamp = now();

   // was there an overun
   if ( lastTime > timestamp )
   {
      elapsedTime = static_cast<time_t>( -1 ) - lastTime + timestamp;

      // it is not very common that the overrun was more than MAX_TIME/2 ago
      // assume in that case, that the lastTime is a timestamp in the (near) future
      if ( elapsedTime > static_cast<time_t>( -1 ) / 2 )
      {
         // the timestamp 'lastTime' was not elapsed
         elapsedTime = 0;
      }
   }
   else
   {
      elapsedTime = ( timestamp - lastTime );
   }

   return elapsedTime;
}

void SystemTime::waitMs( uint16_t ms )
{
   time_t timestamp = now();
   while ( since( timestamp ) < ms )
   {
      if ( ms > 100 )
      {
         Sleep( ms / 2 );
      }
   }
}


#ifdef SUPPORT_CALENDAR

#include <Time/Calender.h>

SIGNAL(RTC_COMP_vect)
{
   RealTimeCounter::setCompareValue(RealTimeCounter::getCompareValue() + SystemTime::S + SystemTime::ticksPerSecondAdjustment);
   Calender::now.addSecond();
}

#endif

void SystemTime::init(ClockSources cs,uint16_t frequency)
{
}

