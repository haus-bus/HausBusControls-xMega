/*
 * HbcConfiguration.h
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#ifndef HwUnits_HomeAutomationConfiguration_H
#define HwUnits_HomeAutomationConfiguration_H

#include "HbcUnits.h"
#include "HwUnitBoards/HwUnitBoards.h"

#include <UserSignature.h>

class Checksum;

class Scheduler;

class HbcConfiguration
{
   public:

      static const uint16_t USER_SIGNATURE_ROW_OFFSET = 2;

      static const uint8_t SIZEOF = 13;
      static const uint8_t MAX_SLOTS = 8;
      static const uint8_t MAX_REPORT_TIME = 60;

      ////    Operations    ////

      uint8_t get( HbcConfiguration& configuration );

      uint16_t getDeviceId();

      inline uint8_t getLogicalButtonMask() const;

      inline uint8_t getReportMemoryStatusTime();

      inline uint8_t getSlotType( uint8_t idx ) const;

      inline uint8_t getStartupDelay() const;

      inline int8_t getTimeCorrectionValue() const;

      inline static HbcConfiguration& instance();

      inline static void restoreFactoryConfiguration( uint8_t id, uint8_t fcke );

      void set( HbcConfiguration& configuration );

      inline void setDeviceId( uint16_t _deviceId );

      inline void setTimeCorrectionValue( int8_t value );

      ////    Attributes    ////

   private:

      uint8_t startupDelay;

      uint8_t logicalButtonMask;

      uint16_t deviceId;

      uint8_t reportMemoryStatusTime;

      uint8_t slotTypes[MAX_SLOTS];

      uint8_t timeCorrectionValue;

      uint8_t reserve;

      uint8_t checksum;
};


inline uint8_t HbcConfiguration::getLogicalButtonMask() const
{
   return UserSignature::read( reinterpret_cast<uintptr_t>( &logicalButtonMask ) );
}

inline uint8_t HbcConfiguration::getReportMemoryStatusTime()
{
   uint8_t value = UserSignature::read( reinterpret_cast<uintptr_t>( &reportMemoryStatusTime ) );

   if ( value > MAX_REPORT_TIME )
   {
      return MAX_REPORT_TIME;
   }
   return value;
}

inline uint8_t HbcConfiguration::getSlotType( uint8_t idx ) const
{
   return UserSignature::read( reinterpret_cast<uintptr_t>( &slotTypes[idx] ) );
}

inline uint8_t HbcConfiguration::getStartupDelay() const
{
   return UserSignature::read( reinterpret_cast<uintptr_t>( &startupDelay ) );
}

inline int8_t HbcConfiguration::getTimeCorrectionValue() const
{
   return UserSignature::read( reinterpret_cast<uintptr_t>( &timeCorrectionValue ) );
}

inline void HbcConfiguration::setTimeCorrectionValue( int8_t value )
{
   UserSignature::write( reinterpret_cast<uintptr_t>( &timeCorrectionValue ), &value, sizeof( value ) );
}

inline HbcConfiguration& HbcConfiguration::instance()
{
   return *reinterpret_cast<HbcConfiguration*>( USER_SIGNATURE_ROW_OFFSET );
}

inline void HbcConfiguration::restoreFactoryConfiguration( uint8_t id, uint8_t fcke )
{
   uint8_t buffer[sizeof( HbcConfiguration ) + 2];
   memset( &buffer, 0xFF, sizeof( buffer ) );
   buffer[0] = id;
   buffer[1] = fcke;
   buffer[2] = 0;
   buffer[3] = 0;
   buffer[4] = LBYTE( 0xFFFF >> CONTROLLER_ID );
   buffer[5] = HBYTE( 0xFFFF >> CONTROLLER_ID );
   UserSignature::write( 0, &buffer, sizeof( buffer ) );
}

inline void HbcConfiguration::setDeviceId( uint16_t _deviceId )
{
   HbcConfiguration conf;
   get( conf );
   conf.deviceId = _deviceId;
   set( conf );
}

#endif
