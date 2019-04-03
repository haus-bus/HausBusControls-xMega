/*
 * Gateway.cpp
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#include "Gateway.h"
#include "IResponse.h"
#include "DebugOptions.h"

#include <Security/Checksum.h>
#include <Scheduler.h>
#include <ErrorMessage.h>

static const uint8_t debugLevel( DEBUG_LEVEL_OFF );

EventDrivenUnit* Gateway::listener( NULL );

uint8_t Gateway::numOfGateways( 0 );

Gateway* Gateway::itsGateway[MAX_GATEWAYS];

void Gateway::addGateway()
{
   for ( uint8_t pos = 0; pos < MAX_GATEWAYS; ++pos )
   {
      if ( !itsGateway[pos] )
      {
         DEBUG_M4( "Gateway: 0x", (uintptr_t )this, " added :", numOfGateways );
         itsGateway[pos] = this;
         numOfGateways++;
         break;
      }
   }
}

void Gateway::terminate()
{
   for ( uint8_t pos = 0; pos < MAX_GATEWAYS; ++pos )
   {
      if ( itsGateway[pos] == this )
      {
         numOfGateways--;
         itsGateway[pos] = NULL;
         Reactive::terminate();
      }
   }
}

bool Gateway::notifyEvent( const Event& event )
{
   if ( event.isEvWakeup() )
   {
      run();
   }
   else if ( event.isEvMessage() )
   {
      DEBUG_H1( FSTR( ".evMessage" ) );
      handleRequest( event.isEvMessage()->getMessage() );
   }
   else if ( event.isEvGatewayMessage() )
   {
      DEBUG_H1( FSTR( ".evGatewayMessage" ) );
      HBCP* message = event.isEvGatewayMessage()->getMessage();
      if ( DebugOptions::gatewaysReadOnly() || !configuration->getOptions().enabled )
      {
         delete message;
      }
      else
      {
         if ( !itsMessageQueue.push( message ) )
         {
            delete message;
            itsMessageQueue.popBack( message );
            delete message;
            notifyError( Gateway::MSG_QUEUE_OVERRUN );
         }
      }
   }
   else if ( event.isEvData() )
   {
      IStream::TransferDescriptor* td = event.isEvData()->getTransferDescriptor();

      if ( td->bytesTransferred && ( td->bytesTransferred <= HBCP::MAX_BUFFER_SIZE ) )
      {
         // we received new data
         DEBUG_H1( FSTR( "->data received" ) );

         HBCP::ControlFrame* msg = (HBCP::ControlFrame*) ( td->pData );

         if ( ( getInstanceId() == UDP_9 ) || ( getInstanceId() == UDP ) )
         {
            if ( *( (uint16_t*) td->pData ) == MAGIC_NUMBER )
            {
               msg = (HBCP::ControlFrame*) ( td->pData + sizeof( MAGIC_NUMBER ) );
               if ( msg->getLength() >= ( td->bytesTransferred - sizeof( MAGIC_NUMBER ) ) )
               {
                  notifyMessageReceived( msg );
               }
            }
         }
         else
         {
            notifyEndOfReadTransfer( td );
         }
      }
   }
   else
   {
      return false;
   }
   return true;
}

void Gateway::run()
{
   if ( inStartUp() )
   {
      IoStream::CommandINIT data;
      data.deviceId = HBCP::getDeviceId() & 0x7F;
      data.buffersize = HBCP::MAX_BUFFER_SIZE;
      data.owner = this;
      if ( configuration && ioStream && ( ioStream->genericCommand( IoStream::INIT, &data ) == IStream::SUCCESS ) )
      {
         // configuration = HwConfiguration::getUdpStreamConfiguration( id );
         SET_STATE_L1( RUNNING );
      }
      else
      {
         Gateway::terminate();
         ErrorMessage::notifyOutOfMemory( id );
      }
   }

   if ( inRunning() )
   {
      uint8_t buffer[HBCP::MAX_BUFFER_SIZE];

      // if there is some data, it will be notified by evData
      ioStream->read( buffer, sizeof( buffer ), this );

      if ( !itsMessageQueue.isEmpty() )
      {
         uint16_t minWaitTime = MIN_IDLE_TIME;

         if ( !itsMessageQueue.isFull() )
         {
            uint8_t remainingCapacity = itsMessageQueue.getCapacity() - itsMessageQueue.getSize();
            if ( remainingCapacity < ( itsMessageQueue.getCapacity() / 4 ) )
            {
               minWaitTime += ( remainingCapacity / 4 );
            }
            else
            {
               minWaitTime += ( HBCP::deviceId % 0x1F ) + ( RETRY_DELAY_TIME * retries ) + 4;
            }
         }

         if ( lastIdleTime.elapsed( minWaitTime ) )
         {
            uint16_t len = itsMessageQueue.front()->getLength();

            if ( len > sizeof( buffer ) )
            {
               ErrorData errorData;
               errorData.bufferOverrun.packetCounter = itsMessageQueue.front()->getControlFrame()->getPacketCounter();
               errorData.bufferOverrun.length = len;
               errorData.bufferOverrun.maxBufferSize = sizeof( buffer );
               notifyError( BUFFER_OVERRUN, (uint8_t*)&errorData );
               itsMessageQueue.pop();
               return;
            }

            // prepare buffer
            memcpy( buffer, itsMessageQueue.front(), len );

            // message->getControlFrame()->setPacketCounter( packetCounter );
            // message->getControlFrame()->encrypt();
            uint8_t* pData = buffer;

            if ( ( getInstanceId() == UDP_9 ) || ( getInstanceId() == UDP ) )
            {
               buffer[2] = LBYTE( MAGIC_NUMBER );
               buffer[3] = HBYTE( MAGIC_NUMBER );
               pData += 2;
               len -= 2;

            }
            else if ( getInstanceId() == RS485 )
            {
               // do not send complete header over the comm line, only checksum
               pData += 3;
               len -= 3;
               *pData = Checksum::get( &pData[1], len - 1 );
            }
            else
            {
               Header* header = (Header*) buffer;
               header->address = 0;
               header->checksum = 0;
               header->lastDeviceId = HBCP::deviceId;
               header->checksum = Checksum::get( buffer, len );
            }
            notifyEndOfWriteTransfer( ioStream->write( pData, len, this ) );
         }
      }
      reportGatewayLoad();
   }
}

void Gateway::notifyEndOfWriteTransfer( IStream::Status status )
{
   writeStatus[retries] = status;
   HBCP* msg;

   if ( ( status != IStream::SUCCESS ) && ( retries < MAX_RETRIES ) )
   {
      if ( ( status == IStream::NO_RECEIVER ) && !retries )
      {
         if ( itsMessageQueue.pop( msg ) )
         {
            delete msg;
         }
      }
      else
      {
         retries++;
         ERROR_2( FSTR( "unable to send : " ), (uint8_t)status );
      }
   }
   else
   {
      if ( itsMessageQueue.pop( msg ) )
      {
         if ( status != IStream::SUCCESS )
         {
            notifyError( Gateway::WRITE_FAILED, (uint8_t*) writeStatus );
         }
         else
         {
            packetCounter++;
            notifyMessageSent( msg );
         }
         delete msg;
      }
      retries = 0;
   }
   lastIdleTime = Timestamp();
}

void Gateway::notifyEndOfReadTransfer( IStream::TransferDescriptor* td )
{
   DEBUG_H1( FSTR( ".EoT Slave" ) );
   uint8_t checksum = -1;
   bool notRelevant = false;
   bool readFailed = false;
   uint16_t transferred = td->bytesTransferred;
   const uint8_t minLength = sizeof( HBCP::ControlFrame ) - HBCP::ControlFrame::DEFAULT_DATA_LENGTH;

   if ( transferred > minLength )
   {
      checksum = Checksum::get( td->pData, transferred );
      uint8_t headerSize = sizeof( HBCP::Header );
      if ( getInstanceId() == RS485 )
      {
         headerSize = 1;
      }
      HBCP::ControlFrame* cf = ( (HBCP::ControlFrame*)&td->pData[headerSize] );
      notRelevant = cf->isFromThisDevice() || ( ( numOfGateways < 2 ) && ( !cf->isRelevantForComponent() || cf->isForBootloader() ) );
      if ( ( checksum == 0 ) && ( transferred == ( cf->getLength() + headerSize ) ) && !notRelevant )
      {
         notifyMessageReceived( cf );
         DEBUG_M1( FSTR( "SUCCESS" ) );
      }
      else
      {
         readFailed = !notRelevant;
      }
   }
   else if ( transferred )
   {
      readFailed = true;
   }

   if ( readFailed )
   {
      ErrorData errorData;
      errorData.readFailed.checksum = checksum;
      errorData.readFailed.transferred = td->bytesTransferred;
      errorData.readFailed.remaining = td->bytesRemaining;
      notifyError( Gateway::READ_FAILED, (uint8_t*) &errorData );

      DEBUG_H2( FSTR( "transferError CS: " ), checksum );
      DEBUG_M2( FSTR( "td->trans  " ), transferred );
      DEBUG_M3( FSTR( "td->remain " ), td->bytesRemaining, endl );
      for ( uint16_t i = 0; i < transferred; i++ )
      {
         DEBUG_L2( '-', td->pData[i] );
      }
   }

   if ( notRelevant )
   {
      DEBUG_M1( FSTR( "not relevant" ) );
   }
   lastIdleTime = Timestamp();
}

uint8_t Gateway::getDebugLevel()
{
   return debugLevel;
}

uint8_t Gateway::getNumOfGateways()
{
   return numOfGateways;
}

void Gateway::notifyError( uint8_t errorCode, uint8_t* errorData )
{

   IResponse event( getId() );

   /*
      // force notification for this gateway, too
      event->header.senderId = 0;

      if( numOfGateways > 1 )
      {
      event->header.senderId = getId();
      }
    */
   event.setErrorCode( errorCode, errorData );
   event.queue();

}

void Gateway::notifyMessageReceived( HBCP::ControlFrame* controlFrame )
{
   DEBUG_H1( FSTR( ".new Msg " ) );
   HBCP* msg = (HBCP*) ( ( (uint8_t*) controlFrame ) - sizeof( HBCP::Header ) );

   HBCP* newMsg = msg->copy();
   if ( newMsg )
   {
      newMsg->header.setSenderId( getId() );
      if ( listener )
      {
         evGatewayMessage( listener, newMsg ).send();
      }

      if ( !evGatewayMessage( Scheduler::getJob( HBCP::SYSTEM_ID ), newMsg ).queue() )
      {
         ERROR_1( FSTR( "EventQueue is full" ) );
         delete newMsg;
      }
   }

   gatewayLoad.messagesPerMinute++;
   gatewayLoad.bytesPerMinute += msg->getLength();
}

void Gateway::notifyMessageSent( HBCP* hacf )
{
   DEBUG_H1( FSTR( "->msg sent" ) );
   gatewayLoad.messagesPerMinute++;
   gatewayLoad.bytesPerMinute += hacf->getLength();
   if ( listener )
   {
      evGatewayMessage( listener, hacf ).send();
   }
}

void Gateway::setNumOfGateways( uint8_t p_numOfGateways )
{

   numOfGateways = p_numOfGateways;

}

void Gateway::Response::setGatewayLoad( GatewayLoad& gatewayLoad )
{
   controlFrame.setDataLength(
      sizeof( getResponse() ) + sizeof( getParameter().gatewayLoad ) );
   setResponse( EVENT_GATEWAY_LOAD );
   getParameter().gatewayLoad = gatewayLoad;
}

void Gateway::reportGatewayLoad()
{
   if ( lastReportTimestamp.since() > SystemTime::MIN )
   {
      if ( DebugOptions::notifyMessageLoad() )
      {
         Response event( getId() );
         event.setGatewayLoad( gatewayLoad );
         event.queue();
      }
      lastReportTimestamp = Timestamp();
      gatewayLoad.bytesPerMinute = 0;
      gatewayLoad.messagesPerMinute = 0;
   }
}

bool Gateway::handleRequest( HBCP* message )
{
   if ( !message->controlFrame.isCommand() )
   {
      return false;
   }
   HBCP::ControlFrame& cf = message->controlFrame;
   Command* data = reinterpret_cast<Command*>( cf.getData() );

   if ( cf.isCommand( Command::SET_CONFIGURATION ) )
   {
      DEBUG_H1( FSTR( ".setConfiguration()" ) );
      configuration->set( data->parameter.setConfiguration );
   }
   else if ( cf.isCommand( Command::GET_CONFIGURATION ) )
   {
      DEBUG_H1( FSTR( ".getConfiguration()" ) );
      Response response( getId(), *message );
      configuration->get( response.setConfiguration().configuration );
      response.queue( getObject( message->header.getSenderId() ) );
   }
   else
   {
      return false;
   }
   return true;
}

Gateway::Response::Parameter& Gateway::Response::setConfiguration()
{
   controlFrame.setDataLength( sizeof( getResponse() ) + sizeof( getParameter().configuration ) );
   setResponse( CONFIGURATION );
   return getParameter();
}