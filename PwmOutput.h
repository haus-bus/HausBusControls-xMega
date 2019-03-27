/*
 * PwmOutput.h
 *
 *  Created on: 17.07.2017
 *      Author: Viktor Pankraz
 */

#ifndef Basics_PwmOutput_H
#define Basics_PwmOutput_H

#include "DigitalOutput.h"

class TimerCounter;


class PwmOutput : public DigitalOutput
{
   ////    Constructors and destructors    ////

   public:

      PwmOutput( uint8_t _portNumber, uint8_t _pinNumber, uint16_t _period );

      ////    Operations    ////

      inline void clear()
      {
         set( 0 );
      }

      uint16_t isSet() const;

      void set( uint16_t value );

      void setPeriode( uint16_t period );

   protected:

      TimerCounter* getTC() const;

};

#endif

