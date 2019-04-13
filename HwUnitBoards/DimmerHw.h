/*
 * DimmerHw.h
 *
 *  Created on: 17.07.2017
 *      Author: Viktor Pankraz
 */

#ifndef Electronics_HwUnitBoards_SlotHw_H
#define Electronics_HwUnitBoards_SlotHw_H

#include "HwUnitBoards.h"
#include "HwUnitBoards/IDimmerHw.h"

#include <PwmOutput.h>



class DimmerHw : public IDimmerHw
{
   public:

      enum Modes
      {
         DIMM_CR = 0,
         DIMM_L = 1,
         SWITCH = 2
      };

      ////    Constructors and destructors    ////

      DimmerHw( PortPin pwmPin, PortPin powerPin, bool _supportsPowerControl );

      ////    Operations    ////

      uint16_t getPwmDuty();

      uint8_t hasError();

      uint16_t isOn();

      void on( uint16_t value );

      virtual uint8_t setMode( uint8_t mode );

      void setPwmDuty( uint16_t duty );

      void stop();

      virtual uint8_t suspend( uint8_t mode );

      ////    Additional operations    ////

      inline uint8_t getPwmEnabled() const
      {
         return pwmEnabled;
      }

      inline void setPwmEnabled( uint8_t p_pwmEnabled )
      {
         pwmEnabled = p_pwmEnabled;
      }

   protected:

      inline static const uint8_t getDebugLevel()
      {
         return debugLevel;
      }

      ////    Attributes    ////

      static const uint8_t debugLevel;

   public:

      uint8_t pwmEnabled : 1;

      uint8_t supportsPowerControl : 1;

      ////    Relations and components    ////

   protected:

      PwmOutput pwmOutput;

      DigitalOutput powerOutput;
};

#endif
