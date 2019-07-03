/*
 * AnalogInputUnit.h
 *
 *  Created on: 22.10.2015
 *      Author: Viktor Pankraz
 */

#ifndef HwUnits_AnalogInputUnit_H
#define HwUnits_AnalogInputUnit_H

#include "HbcUnits.h"
#include "BaseSensorUnit.h"
#include <AnalogInput.h>

class AnalogInputUnit : public BaseSensorUnit
{
   public:

      enum ErrorCodes
      {
         NO_ERROR,
      };

      ////    Constructors and destructors    ////

      AnalogInputUnit( PortPin portPin );

      ////    Operations    ////

   private:

      ////    Additional operations    ////

   public:


   protected:

      HwStatus readMeasurement();

      HwStatus startMeasurement( uint16_t& duration );

      inline static const uint8_t getDebugLevel()
      {
         return debugLevel;
      }

   private:

      ////    Attributes    ////

      AnalogInput analogInput;

   public:

   protected:

      static const uint8_t debugLevel;

   private:



      ////    Relations and components    ////

   protected:

};

#endif
