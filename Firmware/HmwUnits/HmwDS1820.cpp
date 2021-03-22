/*
 * HBWDS1820.cpp
 *
 *  Created on: 26.04.2017
 *      Author: Viktor Pankraz
 */

#include "HmwDS1820.h"
#include "HmwDevice.h"

#include <Security/Crc8.h>
#include <Tracing/Logger.h>
#include <stdlib.h>

#define getId() FSTR( "HmwDS1820 " )

#define INVALID_VALUE -27315
#define ERROR_VALUE -27314

const uint8_t HmwDS1820::debugLevel( DEBUG_LEVEL_OFF );

bool HmwDS1820::selfPowered( true );

HmwDS1820::HmwDS1820( OneWire& _hardware, Config* _config ) :
   hardware( &_hardware ),
   errorCounter( 0 ),
   sendPeer( true ),
   currentCentiCelsius( INVALID_VALUE ),
   lastSentCentiCelsius( 0 )
{
   type = HmwChannel::HMW_DS18X20;
   config = _config;
}

bool HmwDS1820::isSelfPowered()
{
   hardware->reset();
   hardware->sendCommand( READ_POWER_SUPPLY, ( uint8_t* ) &romCode );
   selfPowered = ( hardware->sendReceiveBit( 1 ) ? true : false );
   hardware->reset();
   return selfPowered;
}

bool HmwDS1820::isSensor( uint8_t familiyCode )
{
   return ( ( familiyCode == DS18B20_ID ) || ( familiyCode == DS18S20_ID ) );
}


uint8_t HmwDS1820::get( uint8_t* data )
{
   // MSB first
   *data++ = ( currentCentiCelsius >> 8 );
   *data = currentCentiCelsius & 0xFF;
   return 2;
}

void HmwDS1820::loop()
{
   if ( !isNextActionPending() )
   {
      return;
   }

   if ( getCurrentState() == SEARCH_SENSOR )
   {
      // search for a special sensor
      uint8_t diff = OneWire::SEARCH_FIRST;
      DEBUG_H2( FSTR( " searching for 0x" ), config->id );

      // check for 1-Wire components
      while ( diff != OneWire::LAST_DEVICE )
      {
         uint8_t currentId = 0;
         diff = hardware->searchROM( diff, ( uint8_t* ) &romCode );

         if ( diff == OneWire::PRESENCE_ERROR )
         {
            DEBUG_H1( FSTR( " No devices found" ) );
            break;
         }
         else if ( diff == OneWire::DATA_ERROR )
         {
            ERROR_2( getId(), FSTR( " Bus Error" ) );
            break;
         }
         else
         {
            DEBUG_H1( FSTR( " 0x" ) );

            for ( uint8_t i = 0; i < OneWire::ROMCODE_SIZE; i++ )
            {
               DEBUG_L1( ( ( uint8_t* )&romCode )[i] );
               currentId += ( ( uint8_t* )&romCode )[i];
            }

            if ( isSensor( romCode.family ) )
            {
               DEBUG_L2( FSTR( "->DS18X20, ID:0x" ), currentId );

               if ( ( config->id == 0xFF ) && !isUsed( currentId ) )
               {
                  config->id = currentId;
               }

               if ( config->id == currentId )
               {
                  SET_STATE_L1( START_MEASUREMENT );
                  enable( channelId * 100 );
                  errorCounter = 0;
                  return;
               }
            }
            else
            {
               DEBUG_L1( FSTR( "->UNKNOWN" ) );
            }
         }
      }

      // no sensor found, stop channel
      DEBUG_H1( FSTR( " No sensor for this channel" ) );

      if ( errorCounter >= MAX_ERROR_COUNT ) {
         disable();
      } else {
         setupNextRetry( 5000 );
      }

	  return;	// don't continue (don't send any messages for disabled channels or channels with no sensor)
   }
   else if ( getCurrentState() == START_MEASUREMENT )
   {
	   startMeasurement();	// allSensors = true, so we can only detect error on the entire bus, not the current sensor
	   SET_STATE_L1( READ_MEASUREMENT );
	   enable( 1000 ); // needs about one second to do the measurement
	   
	  // TODO: change to allSensors = false to keep below logic?
	  /*
      if ( startMeasurement() == OK )
      {
         SET_STATE_L1( READ_MEASUREMENT );
         enable( 1000 ); // needs at least one second to do the measurement
         errorCounter = 0;
      }
      else
      {
         setupNextRetry( 1000 );
      }
	  */
   }
   else if ( getCurrentState() == READ_MEASUREMENT )
   {
      if ( readMeasurement() != OK ) {
	     setupNextRetry( 1000 );  // increase errorCounter
	  } else {
		  errorCounter = 0;
		  enable( 5000 );
	  }
      // start next measurement after 5s
      SET_STATE_L1( START_MEASUREMENT );

      if ( errorCounter == MAX_ERROR_COUNT )
      {
         currentCentiCelsius = ERROR_VALUE;
      }
   }
//TODO: as channel loop is only activated when next action is due (isNextActionPending) - the send interval (max-/minInterval) is not accurate - some seconds off... no need to fix?

   bool doSend = ( ( config->maxInterval && ( ( nextFeedbackTime.since() / SystemTime::S ) >= config->maxInterval ) )
                   || ( config->minDelta && ( (uint16_t)labs( currentCentiCelsius - lastSentCentiCelsius ) >= ( (uint16_t)config->minDelta * 10 ) ) ) );

   if ( doSend ) //&& handleFeedback( SystemTime::S* config->minInterval ) )
   {
  #if defined(_Support_HBWLink_InfoEvent_)
   if ( sendPeer )
      {
         uint8_t data[2];
         get( data );
         sendPeer = false;
		 HmwDevice::sendInfoEvent( channelId, data, 2 );
      }
	#endif
      if ( handleFeedback( SystemTime::S* config->minInterval ) )  // sendInfoMessage
      {
  #if defined(_Support_HBWLink_InfoEvent_)
         sendPeer = true;	// feedback (InfoMessage) was send successfully, start with sendInfoEvent again next time "doSend"
	#endif
         lastSentCentiCelsius = currentCentiCelsius;
      }
   }
}

void HmwDS1820::setupNextRetry( uint16_t delay )
{
   if ( errorCounter < MAX_ERROR_COUNT )
   {
      errorCounter++;
   }

   enable( delay * errorCounter ); // each retry makes the wait time longer
}

void HmwDS1820::checkConfig()
{
   if ( config->minDelta > 250 )
   {
      config->minDelta = 5;
   }
   if ( config->minInterval && ( ( config->minInterval < 5 ) || ( config->minInterval > 3600 ) ) )
   {
      config->minInterval = 10;
   }
   if ( config->maxInterval && ( ( config->maxInterval < 5 ) || ( config->maxInterval > 3600 ) ) )
   {
      config->maxInterval = 150;
   }
   if ( config->maxInterval && ( config->maxInterval < config->minInterval ) )
   {
      config->maxInterval = 0;
   }

   // maybe someone has changed the Ids, search the desired sensor now
   if ( config->id != 0 )	// 0 == manually disabled
   {
      SET_STATE_L1( SEARCH_SENSOR );
      nextFeedbackTime = SystemTime::now();
      errorCounter = 0;
      enable( 500 );
   }
   else
   {
      disable();
   }
}


HmwDS1820::HwStatus HmwDS1820::startMeasurement( bool allSensors )
{
   if ( ( hardware->reset() == hardware->NO_ERROR ) && hardware->isIdle() )
   {
      // only send if bus is "idle" = high
      hardware->sendCommand( CONVERT_T, allSensors ? 0 : ( uint8_t* ) &romCode );

      if ( !selfPowered )
      {
         hardware->enableParasite();
      }

      return OK;
   }
   else
   {
      // ERROR_1( "OW not idle on startMeasurement" );
      return START_FAIL;
   }
}

HmwDS1820::HwStatus HmwDS1820::readMeasurement()
{
   uint8_t sp[SCRATCHPAD_SIZE];
   uint8_t isAllZeros = true;

   hardware->reset();
   hardware->sendCommand( READ, ( uint8_t* ) &romCode );

   for ( uint8_t i = 0; i < SCRATCHPAD_SIZE; i++ )
   {
      sp[i] = hardware->read();
	  if ( sp[i] != 0 ) isAllZeros = false;
   }

   if ( Crc8::hasError( sp, SCRATCHPAD_SIZE ) || isAllZeros )  // CRC or read error
   {
      return CRC_FAILURE;
   }

   currentCentiCelsius = convertToCentiCelsius( sp );
   return OK;
}

int16_t HmwDS1820::convertToCentiCelsius( uint8_t* scratchPad )
{
   uint16_t measurement = scratchPad[0];        // LSB
   measurement |= ( (uint16_t) scratchPad[1] ) << 8; // MSB

   // only work on 12bit-base
   if ( romCode.family == DS18S20_ID ) // 9 -> 12 bit if 18S20
   {
      // Extended measurements for DS18S20
      measurement &= (uint16_t) 0xfffe; // Discard LSB , needed for later extended precision calculation
      measurement <<= 3;// Convert to 12-bit , now degrees are in 1/16 degrees units
      measurement += ( 16 - scratchPad[6] ) - 4;// Add the compensation , and remember to subtract 0.25 degree (4/16)
   }

   // clear undefined bits for B != 12bit
   if ( romCode.family == DS18B20_ID ) // check resolution 18B20
   {
      uint8_t i = scratchPad[DS18B20_CONF_REG];
      if ( ( i & DS18B20_12_BIT ) == DS18B20_12_BIT )
      {
      }
      else if ( ( i & DS18B20_11_BIT ) == DS18B20_11_BIT )
      {
         measurement &= ~( DS18B20_11_BIT_UNDF );
      }
      else if ( ( i & DS18B20_10_BIT ) == DS18B20_10_BIT )
      {
         measurement &= ~( DS18B20_10_BIT_UNDF );
      }
      else
      {
         // if ( (i & DS18B20_9_BIT) == DS18B20_9_BIT ) {
         measurement &= ~( DS18B20_9_BIT_UNDF );
      }
   }

   int16_t centiCelsius = static_cast<int8_t>( measurement >> 4 ) * 100;
   uint8_t fracture = (uint8_t)( measurement & 0x000F );
   if ( fracture & 0x8 )
   {
      centiCelsius += 50;
   }
   if ( fracture & 0x4 )
   {
      centiCelsius += 25;
   }
   if ( fracture & 0x2 )
   {
      centiCelsius += 13;
   }
   if ( fracture & 0x1 )
   {
      centiCelsius += 6;
   }

   return centiCelsius;
}


bool HmwDS1820::isUsed( uint8_t id )
{
   for ( uint8_t i = 0; i < HmwChannel::getNumChannels(); i++ )
   {
      HmwChannel* channel = HmwChannel::getChannel( i );

      if ( channel->isOfType( HmwChannel::HMW_DS18X20 ) )
      {
         HmwDS1820* sensor = reinterpret_cast<HmwDS1820*>( channel );

         if ( sensor->getConfigId() == id )
         {
            return true;
         }
      }
   }

   return false;
}