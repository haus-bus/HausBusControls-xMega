/*
 * Dht.h
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#ifndef HwUnits_Dht_H
#define HwUnits_Dht_H

#include "HbcUnits.h"
#include "BaseSensorUnit.h"

#include <Protocols/Dht22.h>

class Event;

class PortPin;

class Scheduler;

class Dht : public BaseSensorUnit
{
   public:

      struct Data
      {
         uint8_t checksum;
         BaseSensorUnit::Status temperature;
         BaseSensorUnit::Status humidity;
      };

      ////    Constructors and destructors    ////

      Dht( uint8_t instanceNumber, PortPin portPin, bool isTemperature );

      ////    Operations    ////

      ////    Additional operations    ////

      inline Dht22* getHardware() const
      {
         return (Dht22*) &hardware;
      }

   protected:

      HwStatus readMeasurement();

      HwStatus startMeasurement( uint16_t& duration );

   private:

      inline static const uint8_t getDebugLevel()
      {
         return debugLevel;
      }

      ////    Attributes    ////

      static const uint8_t debugLevel;

      ////    Relations and components    ////

   protected:

      Dht22 hardware;
};
#endif
