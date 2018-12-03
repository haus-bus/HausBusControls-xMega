/*
 * AR8_Board.cpp
 *
 * Created: 18.06.2014 14:12:55
 * Author: viktor.pankraz
 */

#include "AR8_Board.h"
#include "AR8SystemHw.h"
#include "Enc28j60.h"

#include <Dali.h>
#include <DigitalOutput.h>
#include <PortPin.h>
#include <SlotHw.h>
#include <RS485Hw.h>
#include <Peripherals/DmaChannel.h>
#include <Peripherals/DmaController.h>
#include <Peripherals/EventSystem.h>
#include <Peripherals/IoPort.h>
#include <Release.h>

IrDecoder* irDecoder( 0 );

const ModuleId moduleId = { "$MOD$ AR8      ",
                            0,
                            Release::MAJOR,
                            Release::MINOR,
                            Release::AR8_ID,
                            0 };

SlotHw slotHw[MAX_SLOTS];

DigitalOutput chipSelectSdCard( PortC, 4 );

Enc28j60 enc28j60( Spi::instance( PortC ), DigitalOutput( PortD, 4 ),
                   DigitalInput( PortD, 5 ) );

RS485Hw rs485Hw( Usart::instance<PortE, 0>(), DigitalOutput( PortA, 6 ), DigitalOutput( PortA, 5 ) );

INTERRUPT void USARTE0_RXC_vect()
{
   rs485Hw.handleDataReceived();
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

INTERRUPT void TCC1_CCA_vect()
{
   TRACE_PORT_SET( TR_INT_PIN );
#ifndef _TRACE_PORT_
   notifyBusy();
#endif
   static uint16_t lastStamp = 0;

   // save pulse time
   uint16_t stamp = TCC1.CCA;

   // get edge
   bool edge = ( ( stamp & 0x8000 ) ? true : false );

   // clear edge information
   stamp &= ~0x8000;

   if ( irDecoder )
   {
      irDecoder->notifyEdge( edge, ( stamp - lastStamp ) & 0x7FFF );
   }
   lastStamp = stamp;

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

void configureInfraRedHw( PortPin portPin, IrDecoder* decoder )
{
   DEBUG_M1( FSTR( "IR" ) );
   if ( !irDecoder )
   {
      irDecoder = decoder;

      EventSystem::setEventSource(
         2,
         (EVSYS_CHMUX_t) ( EVSYS_CHMUX_PORTA_PIN0_gc
                           + ( portPin.getPortNumber() << 3 ) + portPin.getPinNumber() ) );

      TimerCounter& t = TimerCounter::instance( PortC, 1 );
      t.configWGM( TC_WGMODE_NORMAL_gc );
      t.setPeriod( 0x7FFF );
#if F_CPU == 32000000UL
      t.configClockSource( TC_CLKSEL_DIV1024_gc );
#elif F_CPU == 8000000UL
      t.configClockSource( TC_CLKSEL_DIV256_gc );
#else
#    error "F_CPU is not supported for IRHardware"
#endif

      t.configInputCapture( TC_EVSEL_CH2_gc );
      t.enableChannels( TC1_CCAEN_bm );
      t.setCCAIntLevel( TC_CCAINTLVL_MED_gc );
   }
}
