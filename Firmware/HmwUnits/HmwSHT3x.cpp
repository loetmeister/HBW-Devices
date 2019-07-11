/*
 * HmwSHT3x.cpp
 * 
 *  Created on: 26.06.2019
 *  by,    loetmeister.de
 *  support Sensirion Humidity Sensor SHT30-D
 * 
 *  Created on: 26.04.2017
 *      Author: Viktor Pankraz
 */

#include "HmwSHT3x.h"
#include "HmwDevice.h"

#include <Security/Crc8.h>
#include <Tracing/Logger.h>
#include <util/delay.h>
#include <stdlib.h>

#define getId() FSTR( "HmwSHT3x " )

#define INVALID_VALUE -27315
#define READ_ERR_COUNT 10 // allow 10 consecutive failures before going to error state (retry is 1 second)

const uint8_t HmwSHT3x::debugLevel( DEBUG_LEVEL_OFF );

HmwSHT3x::HmwSHT3x( Twi& _hardware, Config* _config ) :
   hardware( &_hardware ),
   //isSleeping( true ),
   sendPeer( true ),
   currentHumidity( 0 ),
   lastSentHumidity( 0 ),
   currentCentiCelsius( INVALID_VALUE ),
   lastSentCentiCelsius( INVALID_VALUE )
{
   type = HmwChannel::HMW_SHT3x;
   config = _config;
   lastActionTime = 0;
   SET_STATE_L1( CHECK_SENSOR );
}


uint8_t HmwSHT3x::get( uint8_t* data )
{
   // MSB first
   *data++ = ( currentCentiCelsius >> 8 ) & 0xFF;
   *data++ = currentCentiCelsius & 0xFF;
   *data = currentHumidity;
   return sizeof( currentCentiCelsius ) + sizeof( currentHumidity );
}

void HmwSHT3x::loop( uint8_t channel )
{
   if ( !nextActionDelay || ( lastActionTime.since() < nextActionDelay ) )
   {
      return;
   }

   lastActionTime = Timestamp();   // at least last time of trying

   switch ( state )
   {
      case CHECK_SENSOR:
      {
         if ( checkSensorId() != OK )
         {
            // retry in 10s
            nextActionDelay = 10000;
            return;
         }
		 readMeasurementErrorCounter = READ_ERR_COUNT;	// reset counter
         SET_STATE_L1( START_MEASUREMENT );
      }
      case START_MEASUREMENT:
      {
         if ( startMeasurement() == OK )
         {
            // give some time for measuring before trying to read back the results
            nextActionDelay = 20;
            SET_STATE_L1( READ_MEASUREMENT );
         }
         else
         {
            // retry after 1s
            nextActionDelay = 1000;
         }
         break;
      }

      case READ_MEASUREMENT:
      {
         if ( readMeasurement() != OK )
         {
            // retry after 1s
            nextActionDelay = 1000;
			readMeasurementErrorCounter--;
			if (!readMeasurementErrorCounter)
			{
			// error counter hit - set next state CHECK_SENSOR after 10 consecutive failures (send msg once, then go to CHECK_SENSOR)
				softReset();
				//SET_STATE_L1( CHECK_SENSOR );
				SET_STATE_L1( SEND_FEEDBACK );
				currentCentiCelsius = INVALID_VALUE;
				currentHumidity = 0;
			}
			else {
				SET_STATE_L1( START_MEASUREMENT );
			}
            return;
         }
		 readMeasurementErrorCounter = READ_ERR_COUNT;
         SET_STATE_L1( SEND_FEEDBACK );
      }

      case SEND_FEEDBACK:
      {
         bool doSend = true;

         // do not send before min interval
         doSend &= !( config->minInterval && ( lastSentTime.since() < ( (uint32_t)config->minInterval * 1000 ) ) );
         doSend &= ( ( config->maxInterval && ( lastSentTime.since() >= ( (uint32_t)config->maxInterval * 1000 ) ) )
                   || ( config->minHumidityDelta && ( abs( currentHumidity - lastSentHumidity ) >= config->minHumidityDelta ) )
                   || ( config->minTempDelta && ( abs( currentCentiCelsius - lastSentCentiCelsius ) >= ( (int16_t)config->minTempDelta * 10 ) ) ) );

         if ( doSend )
         {
            uint8_t data[3];
			get( data );
			
			if ( sendPeer )
			{
				sendPeer = false;
				if ( HmwDevice::sendInfoEvent( channel, data, 2 ) == IStream::SUCCESS)	//send temp only
				{
					// at least one peer existed, aad some delay
					nextActionDelay = 500;
					break;
				}
			}
			else {
				uint8_t errcode = HmwDevice::sendInfoMessage( channel, 3, data );
				//uint8_t errcode = HmwDevice::sendInfoMsgAndEvent( channel, data, get( data ) );	// send InfoMessage and InfoEvent at the same time
			
				// sendInfoMessage returns 0 on success, 1 if bus busy, 2 if failed
				if ( errcode != 0 )
				{
				   // retry in 500ms if something fails
				   nextActionDelay = 500;
				   break;
				}
				sendPeer = true;
			}
			
			
            lastSentCentiCelsius = currentCentiCelsius;
			lastSentHumidity = currentHumidity;
            lastSentTime = Timestamp();
         }
         //sleep(); // switch off sensor
		 // sensor lost or in error state
		 if (currentCentiCelsius == INVALID_VALUE) {
			 SET_STATE_L1( CHECK_SENSOR );
			 break;
		 }

         // start next measurement after 5s
         SET_STATE_L1( START_MEASUREMENT );
         nextActionDelay = 5000;
         break;
      }

      default:
      {

      }
   }
}

void HmwSHT3x::checkConfig()
{
   if ( config->minHumidityDelta > 100 )
   {
      config->minHumidityDelta = 2;
   }
   if ( config->minTempDelta > 250 )
   {
      config->minTempDelta = 5;
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
}

HmwSHT3x::HwStatus HmwSHT3x::checkSensorId()
{
   if ( wakeup() == OK )
   {
      if ( sendCommand( SHT3x_CMD_READ_ID ) == OK )
      {
         uint8_t data[DATA_SIZE_READ_ID];
         if ( hardware->master.read( TWI_ADDRESS, &data, sizeof( data ) ) == sizeof( data ) )
         {
            if ( checkCrc( data, 0, data[2] ) == OK )
            {
               uint16_t myId = ( data[0] << 8 ) | data[1];
               if ( myId ) //( ( myId & 0x083F ) == 0x0807 )
               {
                  // yes, this is the expected sensor
                  return OK;
               }
               ERROR_1( FSTR( "Sensor ID does not match" ) );
            }
            ERROR_1( FSTR( "CRC error" ) );
         }
         ERROR_1( FSTR( "TWI read error" ) );
      }
   }
   return FAILTURE;
}

HmwSHT3x::HwStatus HmwSHT3x::startMeasurement()
{
   if ( wakeup() == OK )
   {
      // start measurement
      if ( sendCommand( SHT3x_CMD_CSE_TF_NPM ) == OK )
      {
         return OK;
      }
   }
   return FAILTURE;
}

HmwSHT3x::HwStatus HmwSHT3x::readMeasurement()
{
   if ( wakeup() == OK )
   {
      uint8_t data[DATA_SIZE_READ_TH];
      if ( hardware->master.read( TWI_ADDRESS, &data, sizeof( data ) ) == sizeof( data ) )
      {
         //uint16_t rawData = ( data[0] << 8 ) | data[1];
         if ( checkCrc( data, 0, data[2] ) == OK )
         {
            currentCentiCelsius = convertToCentiCelsius( (uint16_t)( data[0] << 8 ) | data[1] );
            //rawData = ( data[3] << 8 ) | data[4];
            if ( checkCrc( data, 3, data[5] ) == OK )
            {
               currentHumidity = convertToRelativeHumidity( (uint16_t)( data[3] << 8 ) | data[4] );
               return OK;
            }
            ERROR_1( FSTR( "CRC error humidity" ) );
         }
         ERROR_1( FSTR( "CRC error temperature" ) );
      }
      ERROR_1( FSTR( "TWI read error" ) );
   }
   return FAILTURE;
}

int16_t HmwSHT3x::convertToCentiCelsius( uint16_t rawValue )
{
   return ( ( (int32_t)17500 * rawValue ) >> 16 ) - 4500;
}

uint8_t HmwSHT3x::convertToRelativeHumidity( uint16_t rawValue )
{
   return ( ( (uint32_t)100 * rawValue ) >> 16 );
}

HmwSHT3x::HwStatus HmwSHT3x::checkCrc( uint8_t *data, uint8_t _sPos, uint8_t _crc )
{
   uint8_t crc = 0xFF;
   uint8_t poly = 0x31;

   for ( uint8_t indi = _sPos; indi < _sPos +2; indi++ )
   {
      crc ^= data[indi];

      for ( uint8_t indj = 0; indj < 8; indj++ )
      {
         if ( crc & 0x80 )
         {
            crc = (uint8_t)( ( crc << 1 ) ^ poly );
         }
         else
         {
            crc <<= 1;
         }
      }
   }

   return ( _crc ^ crc ) ? FAILTURE : OK;
}

HmwSHT3x::HwStatus HmwSHT3x::sendCommand( HmwSHT3x::Commands cmd )
{
   if ( hardware->master.write( TWI_ADDRESS, &cmd, sizeof( cmd ) ) != sizeof( cmd ) )
   {
      ERROR_3( FSTR( "sendCommand(0x" ), (uint16_t)cmd, FSTR( " failed" ) );
      return FAILTURE;
   }
   return OK;
}

HmwSHT3x::HwStatus HmwSHT3x::softReset()
{
   HwStatus status = sendCommand( SHT3x_CMD_SFT_RST );
   _delay_us( 500 );
   return status;
}

HmwSHT3x::HwStatus HmwSHT3x::wakeup()
{
	////if (supportsSleep?)...
   // HwStatus status = OK;
   // if ( isSleeping )
   // {
      // status = sendCommand( SHT3x_CMD_WAKE );
      // if ( status == OK )
      // {
         // isSleeping = false;
      // }
      // _delay_us( 250 );
   // }
   // return status;
   return OK;
}

 //HmwSHT3x::HwStatus HmwSHT3x::sleep()
 //{
	////if (supportsSleep?)...
    //HwStatus status = OK;
    //if ( !isSleeping )
    //{
       //status = sendCommand( SHT3x_CMD_SLEEP );
       //if ( status == OK )
       //{
          //isSleeping = true;
       //}
    //}
    //return status;
 //}