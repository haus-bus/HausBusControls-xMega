/*
 * Dht22.cpp
 *
 *  Created on: 09.05.2015
 *      Author: viktor.pankraz
 */

#include <Protocols/Dht22.h>

#define getId() FSTR( "Dht22::" )

const uint8_t Dht22::debugLevel( DEBUG_LEVEL_OFF );


Dht22::Dht22( PortPin pin ) :
   ioPin( pin )
{
   ioPin.enablePullup();
}

void Dht22::startMeasurement()
{
   DEBUG_M1( FSTR( "start" ) );
   DigitalOutput out( ioPin );
}

Dht22::Errors Dht22::waitForAck()
{
   CriticalSection doNotInterrupt;

   ioPin.configInput();

   // find the start of the ACK signal
   uint8_t remaining = ioPin.waitPinStateHigh( COUNT_DELAY_BIT_US( ACK_TIMEOUT / 2 ) );
   if ( !remaining )
   {
      DEBUG_M1( FSTR( "waitForAck not present" ) );
      return NOT_PRESENT;
   }

   // find the transition of the ACK signal
   remaining = ioPin.waitPinStateLow( COUNT_DELAY_BIT_US( ACK_TIMEOUT ) );
   if ( !remaining )
   {
      DEBUG_M1( FSTR( "waitForAck 0 too long" ) );
      return ACK_MISSING;
   }

   // find the end of the ACK signal
   remaining = ioPin.waitPinStateHigh( COUNT_DELAY_BIT_US( ACK_TIMEOUT ) );
   if ( !remaining )
   {
      DEBUG_M1( FSTR( "waitForAck 1 too long" ) );
      return ACK_TOO_LONG;
   }

   // find the end of the ACK signal
   remaining = ioPin.waitPinStateLow( COUNT_DELAY_BIT_US( ACK_TIMEOUT ) );
   if ( !remaining )
   {
      DEBUG_M1( FSTR( "waitForDataSync too long" ) );
      return SYNC_TIMEOUT;
   }
   return OK;
}

bool Dht22::isIdle()
{
   // wait if data line is not idle for max 250uSec
   bool idle = ioPin.waitPinStateHigh( COUNT_DELAY_BIT_US( IDLE_TIMEOUT ) );
   idle &= !ioPin.waitPinStateLow( COUNT_DELAY_BIT_US( IDLE_TIMEOUT ) );
   return idle;
}

uint8_t Dht22::read( uint8_t* data )
{
   DEBUG_H1( FSTR( "read" ) );

   CriticalSection doNotInterrupt;

   Errors error = waitForAck();
   if ( error )
   {
      return error;
   }

   uint8_t i = BYTE_COUNT;
   while ( i-- )
   {
      for ( uint8_t j = 0; j < 8; j++ )
      {
         data[i] <<= 1;
         // wait for bit change, record timing
         // the lower phase length is not really specified, just >50us
         uint16_t remainingTime = ioPin.waitPinStateHigh( COUNT_DELAY_BIT_US( 100 ) );
         if ( !remainingTime )
         {
            // we are way out of timing spec
            DEBUG_L4( FSTR( "SYNC_TIMEOUT " ), i, ' ', j );
            return SYNC_TIMEOUT;
         }

         remainingTime = ioPin.waitPinStateLow( COUNT_DELAY_BIT_US( 80 ) );
         if ( !remainingTime )
         {
            // we are way out of timing spec
            DEBUG_L4( FSTR( "DATA_TIMEOUT " ), i, ' ', j );
            return DATA_TIMEOUT;
         }

         if ( remainingTime < COUNT_DELAY_BIT_US( 40 ) )
         {
            data[i] |= 1;
         }
      }
   }
   DEBUG_M1( FSTR( "Data: " ) << data[4] << '-' << data[3] << '-' << data[2] << '-' << data[1] );

   // checksum
   if ( data[0] != (uint8_t) ( data[1] + data[2] + data[3] + data[4] ) )
   {
      DEBUG_M1( FSTR( "CHECKSUM_ERROR" ) );
      return CHECKSUM_ERROR;
   }
   DEBUG_M1( FSTR( "OK" ) );

   return OK;
}
