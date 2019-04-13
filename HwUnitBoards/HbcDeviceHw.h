/*
 * HbcDeviceHw.h
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#ifndef Electronics_SystemBoards_HomeAutomationHw_H
#define Electronics_SystemBoards_HomeAutomationHw_H

#include "HwUnitBoards.h"

#include <ApplicationTable.h>
#include <UserSignature.h>
#include <GlobalInterrupt.h>
#include <IStream.h>

class Checksum;

class Eeprom;

class FlashString;

class Logger;

class ModuleId;

class HbcDeviceHw
{
   public:

      static const uint32_t DBG_BAUDRATE = 115200;

      ////    Constructors and destructors    ////

      inline HbcDeviceHw()
      {
      }

      ////    Operations    ////

      inline void enableInterrupts();

      inline static Flash::address_t findModuleIdPosition( bool loaderModId );

      inline static uint8_t getFirmwareId()
      {
         return UserSignature::read( 0 );
      }

#ifdef _DEBUG_

      static FlashString* getId();

#endif

      static bool getModuleId( uint8_t index, ModuleId* moduleId );

      inline static uint8_t getFckE()
      {
         uint8_t fcke = Flash::readUserSignature( 1 );
         if ( fcke == 0xFF )
         {
            return OLD_BOARDS;
         }
         return fcke;
      }

      inline static uint16_t readRules( uint16_t offset, void* pData, uint16_t length );

      inline static uint16_t writeRules( uint16_t offset, void* pData, uint16_t length );

      ////    Additional operations    ////

   protected:

      inline static const uint8_t getDebugLevel()
      {
         return debugLevel;
      }

      ////    Attributes    ////

      static const uint8_t debugLevel;
};

inline void HbcDeviceHw::enableInterrupts()
{
   PMIC.CTRL |= PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
   GlobalInterrupt::enable();
}

inline Flash::address_t HbcDeviceHw::findModuleIdPosition(
   bool loaderModId )
{
   Flash::address_t address;
   if ( loaderModId )
   {
      address = BOOT_SECTION_START + _VECTORS_SIZE;
   }
   else
   {
      address = APP_SECTION_START + _VECTORS_SIZE;
   }

   if ( Flash::read( address ) == '$' )
   {
      return address - 1;
   }
   return -1;
}

inline uint16_t HbcDeviceHw::readRules( uint16_t offset, void* pData, uint16_t length )
{
   return ApplicationTable::read( offset, pData, length );
}

inline uint16_t HbcDeviceHw::writeRules( uint16_t offset, void* pData, uint16_t length )
{
   if ( length )
   {
      return ApplicationTable::write( offset, pData, length );
   }
   return length;
}

#endif
