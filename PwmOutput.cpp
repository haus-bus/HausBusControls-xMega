/*
 * PwmOutput.cpp
 *
 *  Created on: 17.07.2017
 *      Author: Viktor Pankraz
 */

#include <stddef.h>
#include "PwmOutput.h"
#include "Peripherals/TimerCounter.h"


PwmOutput::PwmOutput( uint8_t _portNumber, uint8_t _pinNumber, uint16_t _period ) : DigitalOutput( _portNumber, _pinNumber )
{
   TimerCounter* tc = getTC();
   if ( tc )
   {
      tc->configClockSource( TC_CLKSEL_DIV1_gc );
      tc->configWGM( TC_WGMODE_SS_gc );
      tc->setPeriod( _period );
      tc->enableChannel( tc->getChannelFromPinNumber( pinNumber ) );
   }
}

uint16_t PwmOutput::isSet() const
{
   TimerCounter* tc = getTC();
   if ( tc )
   {
      return tc->getCapture( tc->getChannelFromPinNumber( pinNumber ) );
   }
   return 0;
}

void PwmOutput::set( uint16_t value )
{
   TimerCounter* tc = getTC();
   if ( tc )
   {
      tc->setCompare( tc->getChannelFromPinNumber( pinNumber ), value );
   }
}

void PwmOutput::setPeriode( uint16_t period )
{
   TimerCounter* tc = getTC();

   if ( tc )
   {
      tc->setPeriod( period );
   }
}

TimerCounter* PwmOutput::getTC() const
{
   if ( pinNumber < 4 )
   {
      return &TimerCounter::instance( portNumber, 0 );
   }
   else if ( pinNumber < 6 )
   {
      return &TimerCounter::instance( portNumber, 1 );
   }
   return NULL;
}

