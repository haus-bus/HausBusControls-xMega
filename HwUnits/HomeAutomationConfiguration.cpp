/*
 * HomeAutomationConfiguration.cpp
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#include "HomeAutomationConfiguration.h"
#include <Security/Checksum.h>


uint16_t HomeAutomationConfiguration::getDeviceId()
{
   uint16_t id = Flash::readUserSignature(
      reinterpret_cast<uint16_t>( &deviceId ) );
   id |= ( Flash::readUserSignature( reinterpret_cast<uint16_t>( &deviceId ) + 1 )
           << 8 );
   if ( ( id == 0 ) || ( id > 0x7FFF ) )
   {
      id = 0x7FFF;
   }
   return id;
}

uint8_t HomeAutomationConfiguration::get( HomeAutomationConfiguration& configuration )
{

   Flash::readUserSignature( reinterpret_cast<uint16_t>( this ), &configuration,
                             sizeof( configuration ) );
   return Checksum::hasError( &configuration, sizeof( configuration ) );

   /*

      Configuration confEeprom;
      Eeprom::read( 0, &confEeprom, sizeof(configuration) );

      if ( Checksum::hasError( &configuration, sizeof(configuration) ) )
      {
      errorCode |= 1;
      }
      if ( Checksum::hasError( &confEeprom, sizeof(confEeprom) ) )
      {
      errorCode |= 2;
      }

      if ( errorCode == 1 )
      {
      set( confEeprom );
      configuration = confEeprom;
      }
      if ( memcmp( &configuration, &confEeprom, sizeof(configuration) ) )
      {
      set( configuration );
      }
    */
}

void HomeAutomationConfiguration::set( HomeAutomationConfiguration& configuration )
{
   configuration.checksum = 0;
   configuration.checksum = Checksum::get( &configuration, sizeof( configuration ) );

   Flash::writeUserSignature( reinterpret_cast<uint16_t>( this ), &configuration, sizeof( configuration ) );
}