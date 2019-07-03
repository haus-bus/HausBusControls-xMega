/*
 * Dht.cpp
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#include "Dht.h"
#include <PortPin.h>

const uint8_t Dht::debugLevel( DEBUG_LEVEL_OFF );

Dht::Dht( uint8_t instanceNumber, PortPin portPin, bool isTemperature ) :
   hardware( portPin )
{
   uint8_t classId = isTemperature ? ClassId::TEMPERATURE : ClassId::HUMIDITY;
   setId( ( classId << 8 ) | instanceNumber );
}

BaseSensorUnit::HwStatus Dht::startMeasurement( uint16_t& duration )
{
   if ( hardware.isIdle() )
   {
      // needs 1 - 10ms before result can be read out
      duration = 5;
      hardware.startMeasurement();
      return BaseSensorUnit::OK;
   }
   // needs minimum 2s before next try can be started
   duration = 3 * SystemTime::S;
   return BaseSensorUnit::START_FAIL;
}

BaseSensorUnit::HwStatus Dht::readMeasurement()
{
   uint8_t rawData[sizeof( Data )];
   uint8_t error = hardware.read( rawData );
   if ( !error )
   {
      Status status;

      if ( isClassId( ClassId::HUMIDITY ) )
      {
         // calculate humidity
         uint16_t help = ( ( rawData[4] << 8 ) + rawData[3] );
         status.value = help / 10;
         status.centiValue = 0;
      }
      else
      {
         // calculate temperature
         uint16_t help = ( ( ( rawData[2] & 0x7F ) << 8 ) + rawData[1] );
         status.value = help / 10;
         status.centiValue = ( help % 10 ) * 10;
         if ( rawData[2] & 0x80 )
         {
            // result is negative
            status.value *= -1;
         }
      }
      notifyNewValue( status );
      return BaseSensorUnit::OK;
   }
   return (BaseSensorUnit::HwStatus)( BaseSensorUnit::BUS_HUNG + error );
}

