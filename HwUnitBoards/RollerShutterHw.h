/*
 * RollerShutterHw.h
 *
 *  Created on: 17.07.2017
 *      Author: Viktor Pankraz
 */

#ifndef HbcUnits_HwUnitBoards_RollerShutterHw_H
#define HbcUnits_HwUnitBoards_RollerShutterHw_H

#include "HwUnitBoards.h"
#include <DigitalOutput.h>


class RollerShutterHw
{
   public:

      enum Direction
      {
         TO_CLOSE,
         TO_OPEN
      };

      class ErrorCode
      {
         public:

            enum Errors {
               NO_ERROR,
               INVALID_MOTOR_STATE,
            };
      };

      inline RollerShutterHw( PortPin directionPin, PortPin powerPin ) :
         direction( false ),
         inverted( false ),
         independent( false ),
         directionOutput( directionPin ),
         powerOutput( powerPin )
      {

      }

      ////    Operations    ////

      bool isDirectionToClose();

      bool isDirectionToOpen();

      void off();

      void on();

      void setDirectionToClose();

      void setDirectionToOpen();

      inline void setInverted( bool _inverted )
      {
         inverted = _inverted;
         if ( !independent )
         {
            directionOutput.setInverted( inverted );
         }
      }

      inline void setIndependent( bool _independent )
      {
         independent = _independent;
      }

   private:

      uint8_t direction : 1;

      uint8_t inverted : 1;

      uint8_t independent : 1;

      DigitalOutput directionOutput;

      DigitalOutput powerOutput;
};
#endif
