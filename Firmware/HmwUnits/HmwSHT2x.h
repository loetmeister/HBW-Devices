/*
 * HmwSHT2x.h
 *
 *  Created on: 26.04.2019
 *  by,    loetmeister.de
 *  support Silicon Labs Si7013, Si7020, Si7021
 *
 * Author: viktor.pankraz
 */

#ifndef HmwSHT2x_H
#define HmwSHT2x_H

#include <Peripherals/Twi.h>
#include <Time/Timestamp.h>
#include <xEeprom.h>

#include "HmwChannel.h"


class HmwSHT2x : public HmwChannel
{
   public:

	static const uint8_t TWI_ADDRESS = 0x40;
	static const uint8_t DATA_SIZE_READ_ID = 0x06;	// Serial_b, two times 2 bytes data + 1 bytes crc
	static const uint8_t DATA_SIZE_READ_TH = 0x03;	// T & H separate readings! 2 bytes for temp or humidity + 1 byte crc

	/* supported sensor types
	* (SHT21 should work, but checkSensorId() need to be changed to accept ID) */
	static const uint8_t SI_7013 = 0x0D;
	static const uint8_t SI_7020 = 0x14;
	static const uint8_t SI_7021 = 0x15;

      enum HwStatus
      {
         OK,
         START_FAIL,
         FAILTURE,
         CRC_FAILTURE,
         OUT_OF_MEMORY
      };

      enum States
      {
         CHECK_SENSOR,
         START_SENSOR,
         START_MEASUREMENT,
         READ_MEASUREMENT,
         STOP_SENSOR,
         SEND_FEEDBACK,
         SEND_INVALID_VALUE
      };
	  
		static const bool MEASURE_HUMIDITY = true;
		static const bool MEASURE_TEMPERATURE = false;

      // Because MSB has to be sent before LSB, the bytes in the following enum are swapped. TWI sends LSB first
      enum Commands
      {
         SHTC2_CMD_RST = 0xFE,						// reset command
         SHTC2_CMD_READ_ID1 = 0x0FFA,				// Read Electronic ID 1st Byte (0xFA0F)
		 SHTC2_CMD_READ_ID2 = 0xC9FC,				// Read Electronic ID 2nd Byte (0xFCC9)
		 SHTC2_CMD_FIRMVERS_CMD = 0xB884,			// Read Firmware Revision (0x84B8)
		 SHTC2_CMD_READRHT_REG = 0xE7,				// Read RH/T User Register 1
		 SHTC2_CMD_MEASRH_NOHOLD = 0xF5,            // Measure Relative Humidity, No Hold Master Mode
		 SHTC2_CMD_MEASTEMP_NOHOLD = 0xF3,          // Measure Temperature, No Hold Master Mode
		 SHTC2_CMD_READPREVTEMP = 0xE0				// Read Temperature Value from Previous RH Measurement (no CRC available)
      };

      struct Config
      {
         uint8_tx minHumidityDelta;
         uint8_tx minTempDelta;
         uint16_tx minInterval;
         uint16_tx maxInterval;
      };

      ////    Constructors and destructors    ////

      HmwSHT2x( Twi& _hardware, Config* _config );

      ////    Operations    ////

      HwStatus softReset();

      HwStatus checkSensorId();

      HwStatus readMeasurement();

      HwStatus startMeasurement();

      // definition of needed functions from HBWChannel class
      virtual uint8_t get( uint8_t* data );
      virtual void loop( uint8_t channel );
      virtual void checkConfig();

   private:
	  
	  inline bool isValidID(uint8_t deviceType) {return ((deviceType == SI_7013) || (deviceType == SI_7020) || (deviceType == SI_7021));}

      inline void setMainState( States _state )
      {
         state = _state;
      }

      HwStatus sendCommand( Commands cmd );

      HwStatus checkCrc( uint8_t *data, uint8_t crc );

      int16_t convertToCentiCelsius( uint16_t rawValue );

      uint8_t convertToRelativeHumidity( uint16_t rawValue );

      ////    Additional operations    ////

   public:


   private:


      ////    Attributes    ////

   public:

      Twi* hardware;

   private:

      static const uint8_t debugLevel;

	  bool measureTempOrHumid;

      States state;

      uint8_t currentHumidity;

      uint8_t lastSentHumidity;

      uint16_t nextActionDelay;

      int16_t currentCentiCelsius;

      int16_t lastSentCentiCelsius;

      Timestamp lastActionTime;

      Timestamp lastSentTime;

      Config* config;
};


#endif

