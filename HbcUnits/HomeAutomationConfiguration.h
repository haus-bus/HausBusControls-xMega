/*
 * HomeAutomationConfiguration.h
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#ifndef HwUnits_HomeAutomationConfiguration_H
#define HwUnits_HomeAutomationConfiguration_H

#include "Electronics.h"
#include "HbcUnits.h"

#include <UserSignature.h>

class Checksum;

class Scheduler;

class HomeAutomationConfiguration
{
   public:

      static const uint16_t USER_SIGNATURE_ROW_OFFSET = 2;

      static const uint8_t SIZEOF = 13;
      static const uint8_t MAX_SLOTS = 8;
      static const uint8_t MAX_REPORT_TIME = 60;

      ////    Operations    ////

      uint8_t get( HomeAutomationConfiguration& configuration );

      uint16_t getDeviceId();

      inline uint8_t getLogicalButtonMask() const;

      inline uint8_t getReportMemoryStatusTime();

      inline uint8_t getSlotType( uint8_t idx ) const;

      inline uint8_t getStartupDelay() const;

      inline int8_t getTimeCorrectionValue() const;

      inline static HomeAutomationConfiguration& instance();

      inline static void restoreFactoryConfiguration( uint8_t id, uint8_t fcke );

      void set( HomeAutomationConfiguration& configuration );

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


inline uint8_t HomeAutomationConfiguration::getLogicalButtonMask() const
{
   return UserSignature::read( reinterpret_cast<uintptr_t>( &logicalButtonMask ) );
}

inline uint8_t HomeAutomationConfiguration::getReportMemoryStatusTime()
{
   uint8_t value = UserSignature::read( reinterpret_cast<uintptr_t>( &reportMemoryStatusTime ) );

   if ( value > MAX_REPORT_TIME )
   {
      return MAX_REPORT_TIME;
   }
   return value;
}

inline uint8_t HomeAutomationConfiguration::getSlotType( uint8_t idx ) const
{
   return UserSignature::read( reinterpret_cast<uintptr_t>( &slotTypes[idx] ) );
}

inline uint8_t HomeAutomationConfiguration::getStartupDelay() const
{
   return UserSignature::read( reinterpret_cast<uintptr_t>( &startupDelay ) );
}

inline int8_t HomeAutomationConfiguration::getTimeCorrectionValue() const
{
   return UserSignature::read( reinterpret_cast<uintptr_t>( &timeCorrectionValue ) );
}

inline void HomeAutomationConfiguration::setTimeCorrectionValue( int8_t value )
{
   UserSignature::write( reinterpret_cast<uintptr_t>( &timeCorrectionValue ), &value, sizeof( value ) );
}

inline HomeAutomationConfiguration& HomeAutomationConfiguration::instance()
{
   return *reinterpret_cast<HomeAutomationConfiguration*>( USER_SIGNATURE_ROW_OFFSET );
}

inline void HomeAutomationConfiguration::restoreFactoryConfiguration( uint8_t id, uint8_t fcke )
{
   uint8_t buffer[sizeof( HomeAutomationConfiguration ) + 2];
   memset( &buffer, 0xFF, sizeof( buffer ) );
   buffer[0] = id;
   buffer[1] = fcke;
   buffer[2] = 0;
   buffer[3] = 0;
   buffer[4] = LBYTE( 0xFFFF >> CONTROLLER_ID );
   buffer[5] = HBYTE( 0xFFFF >> CONTROLLER_ID );
   UserSignature::write( 0, &buffer, sizeof( buffer ) );
}

inline void HomeAutomationConfiguration::setDeviceId( uint16_t _deviceId )
{
   HomeAutomationConfiguration conf;
   get( conf );
   conf.deviceId = _deviceId;
   set( conf );
}

#endif
