/*
 * MS6SystemHw.cpp
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#include "PbsSystemHw.h"
#include <Release.h>
#include <Security/Checksum.h>
#include <HbcConfiguration.h>
#include <Tracing/Logger.h>
#include <LogicalButton.h>
#include <Security/ModuleId.h>
#include <Peripherals/TimerCounter.h>
#include <Peripherals/Usart.h>
#include <Peripherals/WatchDog.h>
#include <Gateway.h>
#include <Protocols/SoftTwi.h>
#include <Protocols/RS485Hw.h>


const ModuleId moduleId = { "$MOD$ PBS      ",
                            0,
                            Release::MAJOR,
                            Release::MINOR,
                            CONTROLLER_ID,
                            0 };

PbsSystemHw::PbsSystemHw()
{
   configure();
}

#ifdef _DEBUG_
void putc( char c )
{
   Usart::instance<DBG_PORT, DBG_CHANNEL>().write( c );
}
#endif

void PbsSystemHw::configure()
{
#ifdef _DEBUG_
   // configure Logger
   Usart::instance<DBG_PORT, DBG_CHANNEL>().init<DBG_BAUDRATE>();
   Logger::instance().setStream( putc );
#endif // _DEBUG_

   // configure ports
   TRACE_PORT_INIT( AllPins );

   DEBUG_H1( FSTR( "configure" ) );
   if ( CONTROLLER_ID == SD485_ID )
   {
      configureRs485();
   }
   else
   {
      IoPort::instance( PortE ).configure( Pin0 | Pin1 | Pin2, PORT_OPC_PULLUP_gc );
      IoPort::instance( PortE ).setPinsAsOutput( Pin3 );
      IoPort::instance( PortE ).setPins( Pin3 );

      IoPort::instance( PortR ).setPinsAsOutput( Pin0 | Pin1 );
      IoPort::instance( PortR ).setPins( Pin0 | Pin1 );
      configureTwi();
   }

   // start Timer of PORTC and PORTD to support LED dimming
   configurePwm( TimerCounter::instance( PortC, 0 ) );
   configurePwm( TimerCounter::instance( PortC, 1 ) );
   configurePwm( TimerCounter::instance( PortD, 0 ) );
   configurePwm( TimerCounter::instance( PortD, 1 ) );

   // enable interrupts
   IoPort::instance<PortE>().enableInterrupt0();
   enableInterrupts();
}

void PbsSystemHw::configureLogicalButtons()
{
   DEBUG_M1( FSTR( "LButtons" ) );

   uint8_t i = 0;
   uint8_t mask = HbcConfiguration::instance().getLogicalButtonMask();
   while ( mask )
   {
      if ( mask & 1 )
      {
         new LogicalButton( i + 1 );
      }
      mask >>= 1;
      i++;
   }
}

void PbsSystemHw::configurePwm( TimerCounter& timer )
{
   #if F_CPU == 32000000UL
   timer.configClockSource( TC_CLKSEL_DIV8_gc );
   #elif F_CPU == 8000000UL
   timer.configClockSource( TC_CLKSEL_DIV1_gc );
   #else
   #    error "F_CPU is not supported for PwmHardware"
   #endif

   timer.configWGM( TC_WGMODE_SS_gc );
   timer.setPeriod( 100 );
}

void notifyBusy()
{
}

void notifyIdle()
{
   #ifndef _DEBUG_
   WatchDog::reset();
   #endif
}

#ifdef SUPPORT_RS485

PortPinTmpl<PortE, 2> rs485RxPin;
RS485Hw rs485Hw( Usart::instance<PortE, 0>(), DigitalOutput( PortDummy, 0 ), DigitalOutput( PortR, 0 ) );

INTERRUPT void USARTE0_RXC_vect()
{
   if ( !rs485Hw.handleDataReceivedFromISR() )
   {
      // transmission competed, enable rx interrupt again
      rs485RxPin.enableInterrupt0Source();
   }
}

INTERRUPT void PORTE_INT0_vect()
{
   // notify new transmission started and disable interrupt
   rs485Hw.notifyRxStartFromISR();
   rs485RxPin.disableInterrupt0Source();
}

void PbsSystemHw::configureRs485()
{
   DEBUG_M1( FSTR( "RS485" ) );
   Usart::configPortPins<PortE, 0>();
   new Gateway( &rs485Hw, Gateway::RS485 );
}
#endif

#ifdef SUPPORT_TWI

SoftTwi twi;

INTERRUPT void PORTE_INT0_vect()
{
   SoftTwi::handleInterrupt0Source();
}

INTERRUPT void PORTE_INT1_vect()
{
   SoftTwi::handleInterrupt1Source();
}

void PbsSystemHw::configureTwi()
{
   new Gateway( &twi, Gateway::TWI );
}

#endif