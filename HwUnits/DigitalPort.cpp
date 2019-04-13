/*
 * DigitalPort.cpp
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#include "DigitalPort.h"
#include "DigitalOutputUnit.h"

#ifdef USE_DHT
#include "Dht.h"
#endif

#ifdef USE_OW
#include "DS1820.h"
#endif

#ifdef USE_IR
#include "IrReceiver.h"
#endif

#ifdef USE_ANALOGIN
#include "AnalogInputUnit.h"
#endif

#ifdef USE_MDIGIN
#include "MonitoredDigitalInput.h"
#endif

#include <ErrorMessage.h>

const uint8_t DigitalPort::debugLevel( DEBUG_LEVEL_OFF );

DigitalPort::Response::Parameter& DigitalPort::Response::setConfiguration()
{
   controlFrame.setDataLength(
      sizeof( getResponse() ) + sizeof( getParameter().configuration ) );
   setResponse( CONFIGURATION );
   return getParameter();
}

bool DigitalPort::notifyEvent( const Event& event )
{
   if ( event.isEvWakeup() )
   {
      run();
   }
   else if ( event.isEvMessage() )
   {
      return handleRequest( event.isEvMessage()->getMessage() );
   }

   return false;
}

void DigitalPort::run()
{
   if ( inStartUp() )
   {
      setConfiguration( ConfigurationManager::getConfiguration<EepromConfiguration>( id ) );
      if ( configuration )
      {
         configureHw();
         SET_STATE_L1( RUNNING );
      }
      else
      {
         terminate();
         ErrorMessage::notifyOutOfMemory( id );
         return;
      }
   }
   else
   {
      uint8_t changedPins = debounce();
      notifyButtonChanges( changedPins );
      setSleepTime( Configuration::RefreshTime );
   }

}

bool DigitalPort::handleRequest( HBCP* message )
{
   bool consumed = true;
   HBCP::ControlFrame& cf = message->controlFrame;
   Command* data = reinterpret_cast<Command*>( cf.getData() );
   if ( cf.isCommand( Command::GET_CONFIGURATION ) )
   {
      DEBUG_H1( FSTR( ".getConfiguration()" ) );
      Response response( getId(), *message );
      configuration->get( response.setConfiguration().configuration );
      response.queue( getObject( message->header.getSenderId() ) );
   }
   else if ( cf.isCommand( Command::SET_CONFIGURATION ) )
   {
      DEBUG_H1( FSTR( ".setConfiguration()" ) );
      configuration->set( data->parameter.setConfiguration );
   }
   else
   {
      return false;
   }

   return consumed;
}

void DigitalPort::clearPinFunction()
{
   uint8_t i = Configuration::MAX_PINS;
   while ( i-- )
   {
      pin[i].button = 0;
   }
}

void DigitalPort::configureHw()
{
   uint8_t subId = getId() & 0xF0;
   uint8_t portNumber = ( subId >> 4 ) - 1;
   uint8_t needsRunning = false;

   uint8_t i = Configuration::MAX_PINS;
   while ( i-- )
   {
      bool notSupported = false;
      uint8_t pinFunction = configuration->getPinFunction( i );
      if ( pinFunction != 0xFF )
      {
         if ( isPinUsable( 1 << i ) )
         {
            if ( pinFunction == ClassId::BUTTON )
            {
               pin[i].button = new Button( subId + i + 1 );
               needsRunning = true;
            }
            else if ( pinFunction == ClassId::COUNTER )
            {
               PortPin( portNumber, i ).enablePullup();
               pin[i].counter = new Counter( subId + i + 1 );
               needsRunning = true;
            }
            else if ( pinFunction == ClassId::LED )
            {
               pin[i].led = new Led( PortPin( portNumber, i ) );
            }
            else if ( pinFunction == ClassId::DIGITAL_OUTPUT )
            {
               new DigitalOutputUnit( PortPin( portNumber, i ) );
            }
#ifdef USE_ANALOGIN
            else if ( pinFunction == ClassId::ANALOG_INPUT )
            {
               // analog inputs has to be high impedance
               new AnalogInputUnit( PortPin( portNumber, i ) );
            }
#endif
#ifdef USE_DHT
            else if ( pinFunction == ClassId::HUMIDITY )
            {
               new Dht( subId + i + 1, PortPin( portNumber, i ) );
            }
#endif
#ifdef USE_IR
            else if ( pinFunction == ClassId::IR_RECEIVER )
            {
               new IrReceiver( subId + i + 1, PortPin( portNumber, i ) );
            }
#endif
#ifdef USE_OW
            else if ( pinFunction == ClassId::TEMPERATURE )
            {
               DS1820::scanAndCreateDevices( PortPin( portNumber, i ) );
            }
#endif
            else
            {
               notSupported = true;
            }
         }
         else
         {
            notSupported = true;
         }
      }
      if ( notSupported )
      {
         Response event( getId() );
         event.setErrorCode( PINFUNCTION_NOT_SUPPORTED + i );
         event.queue();
         configuration->setPinFunction( i, 0xFF );
      }
   }

   if ( !needsRunning )
   {
      setSleepTime( NO_WAKE_UP );
   }
}

uint8_t DigitalPort::debounce()
{
   uint8_t i = state ^ ~hardware->isPinSet( hardware->getInputPins() );   // key changed ?
   counter0 = ~( counter0 & i );                           // reset or count ct0
   counter1 = counter0 ^ ( counter1 & i );                 // reset or count ct1
   i &= counter0 & counter1;                          // count until roll over ?
   state ^= i;                                        // then toggle debounce state

   return i;
}

void DigitalPort::notifyButtonChanges( uint8_t changedPins )
{
   uint8_t i = Configuration::MAX_PINS;
   uint8_t pos = 0x80;
   while ( i-- )
   {
      if ( ( changedPins & pos ) && pin[i].object )
      {
         if ( pin[i].button->isClassId( ClassId::BUTTON ) )
         {
            pin[i].button->updateState( state & pos );
         }
         else if ( pin[i].counter->isClassId( ClassId::COUNTER ) )
         {
            pin[i].counter->updateState( state & pos );
         }
      }
      pos >>= 1;
   }
}
