/*
 * HmwSHT2x.cpp
 *
 *  Created on: 26.04.2019
 *  by,    loetmeister.de
 *  support Silicon Labs Si7013, Si7020, Si7021
 * (SHT21 should work, but checkSensorId() need to be changed to accept ID)
 *
 * Author: viktor.pankraz
 */

#include "HmwSHT2x.h"
#include "HmwDevice.h"

#include <Tracing/Logger.h>
#include <util/delay.h>
#include <stdlib.h>

#define getId() FSTR( "HmwSHT2x " )

const uint8_t HmwSHT2x::debugLevel( DEBUG_LEVEL_OFF );

HmwSHT2x::HmwSHT2x( Twi& _hardware, Config* _config ) :
   hardware( &_hardware ),
   measureTempOrHumid( MEASURE_HUMIDITY ),
   currentHumidity( 0 ),
   lastSentHumidity( 0 ),
   currentCentiCelsius( 0 ),
   lastSentCentiCelsius( 0 )
{
   type = HmwChannel::HMW_SHT2x;
   config = _config;
   lastActionTime = 0;
   SET_STATE_L1( START_SENSOR );
}


uint8_t HmwSHT2x::get( uint8_t* data )
{
   // MSB first
   *data++ = ( currentCentiCelsius >> 8 ) & 0xFF;
   *data++ = currentCentiCelsius & 0xFF;
   *data = currentHumidity;
   return sizeof( currentCentiCelsius ) + sizeof( currentHumidity );
}

void HmwSHT2x::loop( uint8_t channel )
{
   if ( !nextActionDelay || ( lastActionTime.since() < nextActionDelay ) )
   {
      return;
   }

   lastActionTime = Timestamp();   // at least last time of trying

   switch ( state )
   {
      case START_SENSOR:
      {
	      if ( softReset() != OK )
	      {
		      // retry in 10s
		      nextActionDelay = 10000;
		      return;
	      }
		  nextActionDelay = 50;	// give some time for successful reset (min ~15ms)
	      SET_STATE_L1( CHECK_SENSOR );
      }
      case CHECK_SENSOR:
      {
         if ( checkSensorId() != OK )
         {
            // retry in 1s
            nextActionDelay = 1000;
            return;
         }
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
            SET_STATE_L1( START_MEASUREMENT );
            return;
         }
         //SET_STATE_L1( SEND_FEEDBACK );	//-->> readMeasurement() will set next state, after reading H and T!
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
            uint8_t errcode = HmwDevice::sendInfoMessage( channel, get( data ), data );

            // sendInfoMessage returns 0 on success, 1 if bus busy, 2 if failed
            if ( errcode != 0 )
            {
               // retry in 500ms if something fails
               nextActionDelay = 500;
               break;
            }
            lastSentCentiCelsius = currentCentiCelsius;
			lastSentHumidity = currentHumidity;
            lastSentTime = Timestamp();
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

void HmwSHT2x::checkConfig()
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

HmwSHT2x::HwStatus HmwSHT2x::checkSensorId()
{
	if ( sendCommand( SHTC2_CMD_READRHT_REG ) != OK)
		return FAILTURE;
	
	uint8_t data[1];
	if ( hardware->master.read( TWI_ADDRESS, &data, sizeof( data ) ) == sizeof( data ) )
	{
		if ( data[0] == 0x3A )
		{
			_delay_us( 500 );
			if ( sendCommand( SHTC2_CMD_READ_ID2 ) == OK )
			{
				uint8_t dataID[DATA_SIZE_READ_ID];
				if ( hardware->master.read( TWI_ADDRESS, &dataID, sizeof( dataID ) ) == sizeof( dataID ) )
				{
					if ( checkCrc( dataID, dataID[2] ) == OK )
					{
						if ( isValidID(dataID[0]) )
							return OK;
						ERROR_1( FSTR( "Sensor ID does not match" ) );
					}
					ERROR_1( FSTR( "CRC error" ) );
				}
			}
			ERROR_1( FSTR( "TWI read error" ) );
		}
	}
   return FAILTURE;
}

HmwSHT2x::HwStatus HmwSHT2x::startMeasurement()
{
	if (measureTempOrHumid == MEASURE_HUMIDITY)
		return sendCommand( SHTC2_CMD_MEASRH_NOHOLD );
		
	else if (measureTempOrHumid == MEASURE_TEMPERATURE)
		return sendCommand( SHTC2_CMD_MEASTEMP_NOHOLD );
		
	return FAILTURE;
}

HmwSHT2x::HwStatus HmwSHT2x::readMeasurement()
{
    uint8_t data[DATA_SIZE_READ_TH];
	
    if ( hardware->master.read( TWI_ADDRESS, &data, sizeof( data ) ) == sizeof( data ) )
    {
        if ( checkCrc( data, data[2] ) == OK )
        {
			if (measureTempOrHumid == MEASURE_HUMIDITY)
			{
				currentHumidity = convertToRelativeHumidity( (uint16_t)( data[0] << 8 ) | data[1] );
				SET_STATE_L1( START_MEASUREMENT );	// temperature next round
			}
			else if (measureTempOrHumid == MEASURE_TEMPERATURE)
			{
				currentCentiCelsius = convertToCentiCelsius( (uint16_t)( data[0] << 8 ) | data[1] );
				SET_STATE_L1( SEND_FEEDBACK );
			}
			measureTempOrHumid = !measureTempOrHumid;	// alternate temperature and humidity measurement
			return OK;
        }
        ERROR_1( FSTR( "CRC error" ) );
    }
    ERROR_1( FSTR( "TWI read error" ) );
	return FAILTURE;
}

int16_t HmwSHT2x::convertToCentiCelsius( uint16_t rawValue )
{
	uint32_t temperature = rawValue;
	temperature *= 17572;
	temperature /= 65536;
	temperature -= 4685;
	return temperature;
}

uint8_t HmwSHT2x::convertToRelativeHumidity( uint16_t rawValue )
{
	uint32_t humidity = rawValue;
	humidity *= 125;
	humidity /= 65536;
	humidity -= 6;
	return humidity;
}

HmwSHT2x::HwStatus HmwSHT2x::checkCrc( uint8_t *data, uint8_t _crc )
{
    uint8_t crc = 0x00;
    uint16_t poly = 0x131;	//P(x)=x^8+x^5+x^4+1 = 100110001

    for ( uint8_t indi = 0; indi < 2; indi++ )	//uint16_t values only
    {
       crc ^= data[indi];

       for ( uint8_t indj = 0; indj < 8; indj++ )
       {
          if ( crc & 0x80 )
             crc = (uint8_t)( ( crc << 1 ) ^ poly );
          else
             crc <<= 1;
       }
    }

    return ( _crc ^ crc ) ? FAILTURE : OK;
}

HmwSHT2x::HwStatus HmwSHT2x::sendCommand( HmwSHT2x::Commands cmd )
{
   if ( hardware->master.write( TWI_ADDRESS, &cmd, sizeof( cmd ) ) != sizeof( cmd ) )
   {
      ERROR_3( FSTR( "sendCommand(0x" ), (uint16_t)cmd, FSTR( " failed" ) );
      return FAILTURE;
   }
   return OK;
}

HmwSHT2x::HwStatus HmwSHT2x::softReset()
{
	return sendCommand( SHTC2_CMD_RST );
	
   //HwStatus status = sendCommand( SHTC2_CMD_RST );
   //_delay_us( 500 );
   //return status;
}
