/*
 * HBWDS1820.cpp
 *
 *  Created on: 26.04.2017
 *      Author: Viktor Pankraz
 */

#include "HmwSHTC3.h"
#include "HmwDevice.h"

#include <Security/Crc8.h>
#include <Tracing/Logger.h>
#include <util/delay.h>
#include <stdlib.h>

#define getId() FSTR( "HmwSHTC3 " )

const uint8_t HmwSHTC3::debugLevel( DEBUG_LEVEL_OFF );

HmwSHTC3::HmwSHTC3( Twi& _hardware, Config* _config ) :
   hardware( &_hardware ),
   isSleeping( true ),
   currentCentiCelsius( 0 ),
   lastSentCentiCelsius( 0 ),
   config( _config ),
   humidityChannel( NULL )
{
   type = HmwChannel::HMW_SHTC3;
   SET_STATE_L1( CHECK_SENSOR );
   enable( 500 );
}

HmwSHTC3::PassiveHumidity::PassiveHumidity( PassiveHumidity::Config* _config ) :
   currentHumidity( 0 ),
   lastSentHumidity( 0 ),
   config( _config )
{
}


uint8_t HmwSHTC3::get( uint8_t* data )
{
   // MSB first
   *data++ = ( currentCentiCelsius >> 8 ) & 0xFF;
   *data++ = currentCentiCelsius & 0xFF;
   return sizeof( currentCentiCelsius );
}

uint8_t HmwSHTC3::PassiveHumidity::get( uint8_t* data )
{
   // MSB first
   *data++ = currentHumidity;
   return sizeof( currentHumidity );
}

void HmwSHTC3::loop()
{
   if ( !isNextActionPending() )
   {
      return;
   }

   switch ( getCurrentState() )
   {
      case CHECK_SENSOR:
      {
         if ( checkSensorId() != OK )
         {
            // retry in 10s
            enable( 10000 );
            return;
         }
         SET_STATE_L1( START_MEASUREMENT );
      }
      case START_MEASUREMENT:
      {
         if ( startMeasurement() == OK )
         {
            // give some time for measuring before trying to read back the results
            enable( 20 );
            SET_STATE_L1( READ_MEASUREMENT );
         }
         else
         {
            // retry after 1s
            enable( 1000 );
         }
         break;
      }

      case READ_MEASUREMENT:
      {
         if ( readMeasurement() != OK )
         {
            // retry after 1s
            enable( 1000 );
            SET_STATE_L1( START_MEASUREMENT );
            return;
         }
         SET_STATE_L1( SEND_FEEDBACK );
      }

      case SEND_FEEDBACK:
      {
         bool doSend = ( config->maxInterval && ( ( nextFeedbackTime.since() / SystemTime::S ) >= config->maxInterval ) );
         doSend |= ( config->minDelta && ( (uint16_t)labs( currentCentiCelsius - lastSentCentiCelsius ) >= ( (uint16_t)config->minDelta * 10 ) ) );

         if ( doSend && handleFeedback( SystemTime::S* config->minInterval ) )
         {
            lastSentCentiCelsius = currentCentiCelsius;
         }

         sleep(); // switch off sensor

         // start next measurement after 5s
         SET_STATE_L1( START_MEASUREMENT );
         enable( 5000 );
         break;
      }

      default:
      {

      }
   }
}

HmwSHTC3::HwStatus HmwSHTC3::checkSensorId()
{
   if ( wakeup() == OK )
   {
      if ( sendCommand( SHTC3_CMD_READ_ID ) == OK )
      {
         uint8_t data[DATA_SIZE_READ_ID];
         if ( hardware->master.read( TWI_ADDRESS, &data, sizeof( data ) ) == sizeof( data ) )
         {
            uint16_t myId = ( data[0] << 8 ) | data[1];
            if ( checkCrc( myId, data[2] ) == OK )
            {
               if ( ( myId & 0x083F ) == 0x0807 )
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

HmwSHTC3::HwStatus HmwSHTC3::startMeasurement()
{
   if ( wakeup() == OK )
   {
      // start measurement with ClockStretching enabled, TemperaturFirst, NormalPowerMode
      if ( sendCommand( SHTC3_CMD_CSE_TF_NPM ) == OK )
      {
         return OK;
      }
   }
   return FAILTURE;
}

HmwSHTC3::HwStatus HmwSHTC3::readMeasurement()
{
   if ( wakeup() == OK )
   {
      uint8_t data[DATA_SIZE_READ_TH];
      if ( hardware->master.read( TWI_ADDRESS, &data, sizeof( data ) ) == sizeof( data ) )
      {
         uint16_t rawData = ( data[0] << 8 ) | data[1];
         if ( checkCrc( rawData, data[2] ) == OK )
         {
            currentCentiCelsius = convertToCentiCelsius( rawData );
            rawData = ( data[3] << 8 ) | data[4];
            if ( checkCrc( rawData, data[5] ) == OK )
            {
               if ( humidityChannel )
               {
                  humidityChannel->setMeasurementResult( convertToRelativeHumidity( rawData ) );
               }
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

int16_t HmwSHTC3::convertToCentiCelsius( uint16_t rawValue )
{
   return ( ( (int32_t)17500 * rawValue ) >> 16 ) - 4500;
}

uint8_t HmwSHTC3::convertToRelativeHumidity( uint16_t rawValue )
{
   return ( ( (uint32_t)100 * rawValue ) >> 16 );
}

HmwSHTC3::HwStatus HmwSHTC3::checkCrc( uint16_t _value, uint8_t _crc )
{
   uint8_t data[2] = { HBYTE( _value ), LBYTE( _value ) };
   uint8_t crc = 0xFF;
   uint8_t poly = 0x31;

   for ( uint8_t indi = 0; indi < 2; indi++ )
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

HmwSHTC3::HwStatus HmwSHTC3::sendCommand( HmwSHTC3::Commands cmd )
{
   if ( hardware->master.write( TWI_ADDRESS, &cmd, sizeof( cmd ) ) != sizeof( cmd ) )
   {
      ERROR_3( FSTR( "sendCommand(0x" ), (uint16_t)cmd, FSTR( " failed" ) );
      return FAILTURE;
   }
   return OK;
}

HmwSHTC3::HwStatus HmwSHTC3::softReset()
{
   HwStatus status = sendCommand( SHTC3_CMD_SFT_RST );
   _delay_us( 500 );
   return status;
}

HmwSHTC3::HwStatus HmwSHTC3::wakeup()
{
   HwStatus status = OK;
   if ( isSleeping )
   {
      status = sendCommand( SHTC3_CMD_WAKE );
      if ( status == OK )
      {
         isSleeping = false;
      }
      _delay_us( 250 );
   }
   return status;
}

HmwSHTC3::HwStatus HmwSHTC3::sleep()
{
   HwStatus status = OK;
   if ( !isSleeping )
   {
      status = sendCommand( SHTC3_CMD_SLEEP );
      if ( status == OK )
      {
         isSleeping = true;
      }
   }
   return status;
}

void HmwSHTC3::checkConfig()
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

   nextFeedbackTime = SystemTime::now() + SystemTime::S* config->minInterval;
}

void HmwSHTC3::PassiveHumidity::checkConfig()
{
   if ( config->minInterval && ( ( config->minInterval < 5 ) || ( config->minInterval > 3600 ) ) )
   {
      config->minInterval = 10;
   }
   if ( config->minDeltaPercent && ( config->minDeltaPercent > 100 ) )
   {
      config->minDeltaPercent = 2;
   }

   nextFeedbackTime = SystemTime::now() + SystemTime::S* config->minInterval;
}

void HmwSHTC3::PassiveHumidity::setMeasurementResult( uint8_t humidity )
{
   currentHumidity = humidity;
   uint16_t perCentHumidity = perCentOf<uint16_t>( config->minDeltaPercent, lastSentHumidity );

   bool doSend = ( perCentHumidity <= (uint8_t)abs( currentHumidity - lastSentHumidity ) );

   if ( doSend && handleFeedback( SystemTime::S* config->minInterval ) )
   {
      DEBUG_H2( FSTR( "Sending new values: " ), perCentHumidity )
      lastSentHumidity = currentHumidity;
   }
}