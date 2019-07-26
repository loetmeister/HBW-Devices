/*
 * HmwSHT3x.h
 * 
 *  Created on: 26.06.2019
 *  by,    loetmeister.de
 *  support Sensirion Humidity Sensor SHT30-D (and SHT31?)
 * 
 *  Created on: 26.04.2017
 *      Author: Viktor Pankraz
 */

#ifndef HmwSHT3x_H
#define HmwSHT3x_H

#include <Peripherals/Twi.h>
#include <Time/Timestamp.h>
#include <xEeprom.h>

#include "HmwChannel.h"


class HmwSHT3x : public HmwChannel
{
   public:

      static const uint8_t TWI_ADDRESS = 0x44;
      static const uint8_t DATA_SIZE_READ_ID = 0x06;	// 16 bit sn1, 8 bit crc, 16 bit sn2, 8 bit crc
      static const uint8_t DATA_SIZE_READ_TH = 0x06;	// cTemp msb, cTemp lsb, cTemp crc, humidity msb, humidity lsb, humidity crc

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

      // Because MSB has to be sent before LSB, the bytes in the following enum are swapped. TWI sends LSB first
      enum Commands
      {
         SHT3x_CMD_SFT_RST = 0xA230,	// Soft Reset (0x30A2)
         SHT3x_CMD_READ_ID = 0x8037,	// read serial number (0x3780)

         SHT3x_CMD_CSE_TF_NPM = 0x062C,         // clock stretching, high repeatability (0x2C06)
         SHT3x_CMD_READ_SREG = 0x2DF3         // Read Out of status register (0xF32D)
      };

      struct Config
      {
         uint8_tx minHumidityDelta;
         uint8_tx minTempDelta;
         uint16_tx minInterval;
         uint16_tx maxInterval;
      };

      ////    Constructors and destructors    ////

      HmwSHT3x( Twi& _hardware, Config* _config );

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

      inline void setMainState( States _state )
      {
         state = _state;
      }

      HwStatus sendCommand( Commands cmd );

      // HwStatus sleep();

      HwStatus wakeup();

      HwStatus checkCrc( uint8_t *data, uint8_t _sPos, uint8_t _crc );

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

      //bool isSleeping;
	  uint8_t readMeasurementErrorCounter;
	  
	  bool sendPeer;
	  
      States state;

      uint8_t currentHumidity;

      uint8_t lastSentHumidity;

      uint16_t nextActionDelay;

      int16_t currentCentiCelsius;

      int16_t lastSentCentiCelsius;

      Timestamp lastActionTime;

      //Timestamp lastSentTime;

      Config* config;
};


#endif

