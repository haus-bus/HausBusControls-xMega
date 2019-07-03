/*
 * Dht22.h
 *
 *  Created on: 09.05.2015
 *      Author: viktor.pankraz
 */

#ifndef HwProtocols_Dht22_H
#define HwProtocols_Dht22_H

#include "Protocols.h"

#include <Time/SystemTime.h>
#include <DigitalOutput.h>

class Dht22
{
   public:

      static const uint8_t BYTE_COUNT = 5;
      static const uint8_t IDLE_TIMEOUT = 250;
      static const uint8_t ACK_TIMEOUT = 100;

      enum Errors
      {
         OK,
         BUS_HUNG,
         NOT_PRESENT,
         ACK_TOO_LONG,
         SYNC_TIMEOUT,
         DATA_TIMEOUT,
         CHECKSUM_ERROR,
         ACK_MISSING
      };

      ////    Constructors and destructors    ////

      Dht22( PortPin pin );

      ////    Operations    ////

      void startMeasurement();

      uint8_t read( uint8_t* data );

      bool isIdle();

   private:

      Errors waitForAck();

      ////    Additional operations    ////

   public:

      inline PortPin* getPortPin() const
      {
         return (PortPin*) &ioPin;
      }

   private:

      inline static const uint8_t getDebugLevel()
      {
         return debugLevel;
      }

      ////    Attributes    ////

   public:

      PortPin ioPin;

   private:

      static const uint8_t debugLevel;
};

#endif
