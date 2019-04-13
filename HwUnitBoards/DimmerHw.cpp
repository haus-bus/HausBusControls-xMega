/*
 * DimmerHw.cpp
 *
 * Created: 18.06.2014 14:12:55
 * Author: viktor.pankraz
 */

#include "DimmerHw.h"
#include <Tracing/Logger.h>

const uint8_t DimmerHw::debugLevel( DEBUG_LEVEL_OFF );

DimmerHw::DimmerHw( PortPin pwmPin, PortPin powerPin, bool _supportsPowerControl ) :
   pwmEnabled( 0 ),
   supportsPowerControl( _supportsPowerControl ),
   pwmOutput( pwmPin ),
   powerOutput( powerPin )
{
}

uint16_t DimmerHw::getPwmDuty()
{
   uint16_t compareValue = pwmOutput.isSet();

   if ( compareValue < pwmOutput.getPeriode() )
   {
      return compareValue / ( pwmOutput.getPeriode() / 1000 );
   }
   return 1000;
}

uint8_t DimmerHw::hasError()
{
   if ( !supportsPowerControl && powerOutput.isSet() )
   {
      return ErrorCode::DEFECT;
   }
   if ( supportsPowerControl && isOn() && !powerOutput.isSet() )
   {
      return ErrorCode::DEFECT;
   }
   if ( pwmOutput.getPeriode() == 0xFFFF )
   {
      return ErrorCode::INVALID_PERIOD;
   }
   if ( !pwmOutput.isRunning() && pwmEnabled )
   {
      return ErrorCode::NO_ZERO_CROSS_DETECTED;
   }
   return ErrorCode::NO_FAILTURE;
}

uint16_t DimmerHw::isOn()
{
   if ( pwmEnabled && pwmOutput.isRunning() )
   {
      return getPwmDuty();
   }
   return powerOutput.isSet();
}

void DimmerHw::on( uint16_t value )
{
   setPwmDuty( value );
}

uint8_t DimmerHw::setMode( uint8_t mode )
{
   if ( mode == DIMM_L )
   {
      pwmOutput.setInverted( true );
   }
   else
   {
      pwmOutput.setInverted( false );
   }
   pwmEnabled = ( mode < SWITCH );
   return ( mode > SWITCH ) ? ErrorCode::INVALID_MODE : ErrorCode::NO_FAILTURE;
}

void DimmerHw::setPwmDuty( uint16_t duty )
{
   if ( pwmEnabled )
   {
      if ( pwmOutput.isInverted() )
      {
         if ( duty > 1000 )
         {
            duty = 0;
         }
         else
         {
            duty = 1000 - duty;
         }
      }
      uint16_t compareValue = ( pwmOutput.getPeriode() + 500 ) / 1000 * duty;
      DEBUG_M1( compareValue );

      if ( duty )
      {
         if ( pwmOutput.isRunning() && ( duty <= 1000 ) )
         {
            pwmOutput.set( compareValue );
         }
         else
         {
            pwmOutput.DigitalOutput::set();
         }
      }
      else
      {
         pwmOutput.clear();
      }
   }
   else
   {
      if ( duty )
      {
         pwmOutput.DigitalOutput::set();
      }
      else
      {
         pwmOutput.clear();
         pwmOutput.DigitalOutput::clear();
      }
   }
}

void DimmerHw::stop()
{
   pwmOutput.clear();
   pwmOutput.isInverted() ? pwmOutput.DigitalOutput::set() : pwmOutput.DigitalOutput::clear();
   powerOutput.clear();
}

uint8_t DimmerHw::suspend( uint8_t mode )
{
   if ( supportsPowerControl )
   {
      if ( mode == ENABLE )
      {
         powerOutput.configOutput();
         powerOutput.clear();
      }
      else if ( mode == DISABLE )
      {
         powerOutput.configOutput();
         powerOutput.set();
      }
      else
      {
         powerOutput.configInput();
      }
   }
   return true;
}
