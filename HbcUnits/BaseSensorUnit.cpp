/*
 * BaseSensorUnit.cpp
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#include "BaseSensorUnit.h"

const uint8_t BaseSensorUnit::debugLevel( DEBUG_LEVEL_OFF );

BaseSensorUnit::Response::Parameter& BaseSensorUnit::Response::setConfiguration()
{
   controlFrame.setDataLength( sizeof( getResponse() ) + sizeof( getParameter().configuration ) );
   setResponse( CONFIGURATION );
   return getParameter();
}

void BaseSensorUnit::Response::setStatus( BaseSensorUnit::Status value )
{
   controlFrame.setDataLength( sizeof( getResponse() ) + sizeof( getParameter().status ) );
   setResponse( STATUS );
   getParameter().status = value;
}

uint16_t BaseSensorUnit::getMeasurementInterval()
{
   if ( configuration && configuration->reportTimeBase )
   {
      return SystemTime::S* configuration->reportTimeBase;
   }
   return SystemTime::S* Configuration::DEFAULT_REPORT_TIME_BASE;
}

bool BaseSensorUnit::notifyEvent( const Event& event )
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

void BaseSensorUnit::run()
{
   if ( inStartUp() )
   {
      setConfiguration( ConfigurationManager::getConfiguration<EepromConfiguration>( id ) );
      if ( configuration )
      {
         SET_STATE_L1( IDLE );
      }
      else
      {
         terminate();
         ErrorMessage::notifyOutOfMemory( id );
         return;
      }
   }
   else if ( inIdle() )
   {
      errorCounter = 0;
      SET_STATE_L1( RUNNING );
      SET_STATE_L2( START_MEASUREMENT );
   }
   else if ( inRunning() )
   {
      Response event( getId() );
      HwStatus hwStatus;

      if ( inSubState( START_MEASUREMENT ) )
      {
         hwStatus = startMeasurement( lastMeasurementDuration );
         if ( hwStatus == OK )
         {
            SET_STATE_L2( READ_MEASURMENT );
         }
         setSleepTime( lastMeasurementDuration );
      }
      else // READ_MEASURMENT
      {
         hwStatus = readMeasurement();
         setSleepTime( getMeasurementInterval() - lastMeasurementDuration );
         SET_STATE_L2( START_MEASUREMENT );
      }

      if ( hwStatus == OK )
      {
         errorCounter = 0;
      }
      else
      {
         errorCounter++;
         WARN_1( getId() << FSTR( " HwStatus not OK! " ) << (uint8_t)hwStatus );
         if ( ( errorCounter % MAX_ERRORS ) == 0 )
         {
            uint8_t errorData[IResponse::MAX_ERROR_DATA];
            memset( errorData, 0, sizeof( errorData ) );
            errorData[0] = errorCounter;
            event.setErrorCode( hwStatus, errorData );
            event.queue();
         }
         else if ( errorCounter > 4 * MAX_ERRORS )
         {
            // too many errors have occurred, go into IDLE state until GET_STATUS is called
            SET_STATE_L1( IDLE );
            setSleepTime( NO_WAKE_UP );
         }

      }
   }

}

void BaseSensorUnit::notifyNewValue( BaseSensorUnit::Status newStatus )
{
   if ( !configuration || !configuration->reportTimeBase )
   {
      lastStatus = newStatus;
      return;
   }

   bool initialRun = ( currentEvent == 0 );
   uint8_t nextEvent = currentEvent;

   int16_t newStatusValue = newStatus.getCompleteValue();
   int16_t lowerThreshold = configuration->lowerThreshold * 100 + configuration->lowerThresholdFraction;
   int16_t upperThreshold = configuration->upperThreshold * 100 + configuration->upperThresholdFraction;
   int16_t hysteresis = configuration->hysteresis * 10;

   // | old | new | Event
   // ---------------------
   // |  b  |  b  |
   // |  b  |  r  | evInRange
   // |  b  |  a  | evAbove
   // |  r  |  b  | evBelow
   // |  r  |  r  |
   // |  r  |  a  | evAbove
   // |  a  |  b  | evBelow
   // |  a  |  r  | evInRange
   // |  a  |  a  |
   //
   // b, value is < lower
   // r, value is >= lower && < upper
   // a, value is >= upper

   if ( ( hysteresis == 0 ) || initialRun )
   {
      // this is the first value after reset or hysteresis is disabled
      if ( newStatusValue < lowerThreshold )
      {
         nextEvent = Response::EVENT_BELOW;
      }
      else if ( newStatusValue >= upperThreshold )
      {
         nextEvent = Response::EVENT_ABOVE;
      }
      else
      {
         nextEvent = Response::EVENT_IN_RANGE;
      }
   }
   else // hysteresis != 0
   {
      if ( newStatusValue < lowerThreshold )
      {
         if ( lastEvent != Response::EVENT_BELOW )
         {
            nextEvent = Response::EVENT_BELOW;
         }
         else if ( newStatusValue < ( lowerThreshold - hysteresis ) )
         {
            nextEvent = Response::EVENT_BELOW;
         }
      }
      else if ( newStatusValue >= upperThreshold )
      {
         if ( lastEvent != Response::EVENT_ABOVE )
         {
            nextEvent = Response::EVENT_ABOVE;
         }
         else if ( newStatusValue > ( upperThreshold + hysteresis ) )
         {
            nextEvent = Response::EVENT_ABOVE;
         }
      }
      else
      {
         if ( lastEvent != Response::EVENT_IN_RANGE )
         {
            nextEvent = Response::EVENT_IN_RANGE;
         }
         else if ( ( newStatusValue > ( lowerThreshold + hysteresis ) )
                 || ( newStatusValue < ( upperThreshold - hysteresis ) ) )
         {
            nextEvent = Response::EVENT_IN_RANGE;
         }
      }
   }

   if ( nextEvent != currentEvent )
   {
      if ( !initialRun )
      {
         Response event( getId() );
         event.setEvent( nextEvent );
         event.queue();
      }
      lastEvent = currentEvent;
      currentEvent = nextEvent;
   }
   newStatus.lastEvent = currentEvent;
   currentStatus = newStatus;

   if ( initialRun )
   {
      lastStatus = newStatus;
      timeSinceReport = configuration->maxReportTime;
   }
   else
   {
      timeSinceReport++;
      if ( timeSinceReport > configuration->minReportTime )
      {
         if ( ( timeSinceReport > configuration->maxReportTime )
            || ( abs( lastStatus.getCompleteValue() - newStatusValue ) > hysteresis ) )
         {
            timeSinceReport = 0;
            Response event( getId() );
            event.setStatus( newStatus );
            event.queue();
            lastStatus = newStatus;
         }
      }
   }
}

bool BaseSensorUnit::handleRequest( HBCP* message )
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
      timeSinceReport = configuration->maxReportTime;
   }
   else
   {
      Response response( getId(), *message );
      if ( cf.isCommand( Command::GET_CONFIGURATION ) )
      {
         DEBUG_H1( FSTR( ".getConfiguration()" ) );
         configuration->get( response.setConfiguration().configuration );
      }
      else if ( cf.isCommand( Command::GET_STATUS ) )
      {
         DEBUG_H1( FSTR( ".getStatus()" ) );
         response.setStatus( currentStatus );
      }
      else
      {
         return false;
      }
      response.queue( getObject( message->header.getSenderId() ) );
   }

   // check for each communication if sensor is in IDLE state because of an error
   if ( inIdle() )
   {
      // wake up sensor and try to start a new measurement
      setSleepTime( WAKE_UP );
   }

   return true;
}
