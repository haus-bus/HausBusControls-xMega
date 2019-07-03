/*
 * AR8_Board.cpp
 *
 * Created: 18.06.2014 14:12:55
 * Author: viktor.pankraz
 */

#include "AR8_Board.h"
#include "AR8SystemHw.h"
#include "Enc28j60.h"

#include <DigitalOutput.h>
#include <PortPin.h>
#include <Protocols/RS485Hw.h>
#include <Protocols/SoftTwi.h>
#include <Peripherals/DmaChannel.h>
#include <Peripherals/DmaController.h>
#include <Peripherals/EventSystem.h>
#include <Peripherals/IoPort.h>
#include <Peripherals/WatchDog.h>
#include <Peripherals/Usart.h>
#include <Release.h>

const ModuleId moduleId = { "$MOD$ AR8      ",
                            0,
                            Release::MAJOR,
                            Release::MINOR,
                            AR8_ID,
                            0 };

DigitalOutput chipSelectSdCard( PortC, 4 );

DigitalOutput redLed( PortR, 0 );
DigitalOutput greenLed( PortR, 1 );

Enc28j60 enc28j60( Spi::instance( PortC ), DigitalOutput( PortD, 4 ), DigitalInput( PortD, 5 ) );

RS485Hw rs485Hw( Usart::instance<PortE, 0>(), DigitalOutput( PortA, 6 ), DigitalOutput( PortA, 5 ) );

void notifyBusy()
{
   TRACE_PORT_SET( TR_IDLE_PIN );
   greenLed.set();
}

void notifyIdle()
{
   TRACE_PORT_CLEAR( TR_IDLE_PIN );
   greenLed.clear();

   #ifndef _DEBUG_
   WatchDog::reset();
   #endif
}

PortPinTmpl<PortE, 0> twiSdaPin;
PortPinTmpl<PortE, 1> twiSclPin;
PortPinTmpl<PortE, 2> rs485RxPin;

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
   if ( HbcDeviceHw::getFckE() >= FCKE_V4_0 )
   {
      // notify new transmission started and disable interrupt
      rs485Hw.notifyRxStartFromISR();
      rs485RxPin.disableInterrupt0Source();
   }
   else
   {
      SoftTwi::handleInterrupt0Source();
   }
}

INTERRUPT void PORTE_INT1_vect()
{
   SoftTwi::handleInterrupt1Source();
}

INTERRUPT void PORTA_INT0_vect()
{

   TRACE_PORT_SET( TR_INT_PIN );
#ifndef _TRACE_PORT_
   notifyBusy();
#endif

   // check for ZeroCrossDetection is OK
#if F_CPU == 32000000UL
   TimerCounter::instance( PortC, 0 ).configClockSource( TC_CLKSEL_DIV8_gc );
   TimerCounter::instance( PortD, 0 ).configClockSource( TC_CLKSEL_DIV8_gc );
#elif F_CPU == 8000000UL
   TimerCounter::instance( PortC, 0 ).configClockSource( TC_CLKSEL_DIV2_gc );
   TimerCounter::instance( PortD, 0 ).configClockSource( TC_CLKSEL_DIV2_gc );
#else
#    error "F_CPU is not supported for ZeroCrossDetection"
#endif

   TRACE_PORT_CLEAR( TR_INT_PIN );
}

INTERRUPT void TCD1_OVF_vect()
{

   TRACE_PORT_SET( TR_INT_PIN );
#ifndef _TRACE_PORT_
   notifyBusy();
#endif

   // if we get here, zero cross detection fails
   TimerCounter::instance( PortC, 0 ).configClockSource( TC_CLKSEL_OFF_gc );
   TimerCounter::instance( PortD, 0 ).configClockSource( TC_CLKSEL_OFF_gc );

   TRACE_PORT_CLEAR( TR_INT_PIN );
}