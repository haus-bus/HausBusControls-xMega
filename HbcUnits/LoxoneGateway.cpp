/*
 * LoxoneUdp.cpp
 *
 *  Created on: 15.05.2019
 *      Author: Viktor Pankraz
 */

#include "LoxoneGateway.h"

#include <BaseSensorUnit.h>
#include <HwUnits/Button.h>
#include <HwUnits/Led.h>
#include <HwUnits/Dimmer.h>

const uint8_t LoxoneGateway::debugLevel( DEBUG_LEVEL_LOW );

const char LoxoneGateway::MSG_TAG[] = ".";


bool LoxoneGateway::notifyEvent( const Event& event )
{
   // incoming data has to be translated from LOXONE to HBC protocol
   // everything else should be handled by the base class Gateway
   if ( event.isEvData() )
   {
      IStream::TransferDescriptor* td = event.isEvData()->getTransferDescriptor();

      if ( td->bytesTransferred && ( td->bytesTransferred <= ( HBCP::MAX_BUFFER_SIZE - 1 ) ) )
      {
         // we received new data
         DEBUG_H1( FSTR( "->data received" ) );
         char* loxoneMsg = (char*)td->pData;

         // terminate string with \0 (if not already done by Loxone)
         loxoneMsg[td->bytesTransferred] = 0;
         DEBUG_M1( loxoneMsg );

         // get first parameter
         char* parameter = strtok( loxoneMsg, MSG_TAG );

         // loop through all the rest of data
         uint8_t i = 0;
         uint8_t classId = ClassId::UNKNOWN;
         IResponse cmd( getId() );

         while ( parameter != NULL )
         {
            switch ( i )
            {
               case IDX_DEVICE_ID:
               {
                  cmd.controlFrame.receiverId.setDeviceId( atol( parameter ) );
                  break;
               }
               case IDX_CLASS_ID:
               {
                  if ( strcmp( parameter, "LED" ) == 0 )
                  {
                     classId = ClassId::LED;
                  }
                  else if ( strcmp( parameter, "OUT" ) == 0 )
                  {
                     classId = ClassId::DIGITAL_OUTPUT;
                  }
                  else if ( strcmp( parameter, "DIM" ) == 0 )
                  {
                     classId = ClassId::DIMMER;
                  }
                  else
                  {
                     WARN_2( FSTR( "LoxoneGateway::evData() invalid classId: " ), parameter );
                     return false;
                  }
                  cmd.controlFrame.receiverId.setClassId( classId );
                  break;
               }
               case IDX_INSTANCE_ID:
               {
                  cmd.controlFrame.receiverId.setInstanceId( atoi( parameter ) );
                  break;
               }
               case IDX_COMMAND:
               {
                  bool isInvalid = false;

                  if ( classId == ClassId::LED )
                  {
                     if ( strcmp( parameter, "ON" ) == 0 )
                     {
                        cmd.controlFrame.setDataLength( 1 + sizeof( Led::Command::On ) );
                        cmd.controlFrame.resetData();
                        cmd.controlFrame.setCommand( Led::Command::ON );

                        Led::Command* cmdOn = (Led::Command*)cmd.controlFrame.getData();

                        // brightness (byte)
                        cmdOn->parameter.on.brightness = 100;
                        parameter = strtok( NULL, MSG_TAG );
                        i++;
                        if ( parameter )
                        {
                           cmdOn->parameter.on.brightness = atoi( parameter );

                           // duration (word)
                           parameter = strtok( NULL, MSG_TAG );
                           i++;
                           if ( parameter )
                           {
                              cmdOn->parameter.on.duration = atol( parameter );
                           }
                        }

                     }
                     else if ( strcmp( parameter, "OFF" ) == 0 )
                     {
                        cmd.controlFrame.setDataLength( 1 );
                        cmd.controlFrame.setCommand( Led::Command::OFF );
                     }
                     else
                     {
                        isInvalid = true;
                     }
                  }
                  else if ( classId == ClassId::DIGITAL_OUTPUT )
                  {
                     if ( strcmp( parameter, "ON" ) == 0 )
                     {
                        cmd.controlFrame.setDataLength( 1 + sizeof( PortPinUnit::Command::On ) );
                        cmd.controlFrame.resetData();
                        cmd.controlFrame.setCommand( PortPinUnit::Command::ON );

                        PortPinUnit::Command* cmdOn = (PortPinUnit::Command*)cmd.controlFrame.getData();

                        // duration (word)
                        parameter = strtok( NULL, MSG_TAG );
                        i++;
                        if ( parameter )
                        {
                           cmdOn->parameter.on.duration = atol( parameter );
                        }
                     }
                     else if ( strcmp( parameter, "OFF" ) == 0 )
                     {
                        cmd.controlFrame.setDataLength( 1 );
                        cmd.controlFrame.setCommand( PortPinUnit::Command::OFF );
                     }
                     else
                     {
                        isInvalid = true;
                     }
                  }
                  else if ( classId == ClassId::DIMMER )
                  {
                     if ( strcmp( parameter, "ON" ) == 0 )
                     {
                        cmd.controlFrame.setDataLength( 1 + sizeof( Dimmer::Command::SetBrightness ) );
                        cmd.controlFrame.resetData();
                        cmd.controlFrame.setCommand( Dimmer::Command::SET_BRIGHTNESS );

                        Dimmer::Command* cmdOn = (Dimmer::Command*)cmd.controlFrame.getData();

                        // brightness (byte)
                        cmdOn->parameter.setBrightness.brightness = 100;
                        parameter = strtok( NULL, MSG_TAG );
                        i++;
                        if ( parameter )
                        {
                           cmdOn->parameter.setBrightness.brightness = atoi( parameter );

                           // duration (word)
                           parameter = strtok( NULL, MSG_TAG );
                           i++;
                           if ( parameter )
                           {
                              cmdOn->parameter.setBrightness.duration = atol( parameter );
                           }
                        }

                     }
                     else if ( strcmp( parameter, "OFF" ) == 0 )
                     {
                        cmd.controlFrame.setDataLength( 1 + sizeof( Dimmer::Command::SetBrightness ) );
                        cmd.controlFrame.resetData();
                        cmd.controlFrame.setCommand( Dimmer::Command::SET_BRIGHTNESS );
                     }
                     else if ( strcmp( parameter, "START" ) == 0 )
                     {
                        cmd.controlFrame.setDataLength( 1 + sizeof( Dimmer::Command::Start ) );
                        cmd.controlFrame.setCommand( Dimmer::Command::START );

                        Dimmer::Command* cmdStart = (Dimmer::Command*)cmd.controlFrame.getData();

                        // direction ( sbyte)
                        parameter = strtok( NULL, MSG_TAG );
                        i++;
                        if ( parameter )
                        {
                           cmdStart->parameter.start.direction = atoi( parameter );
                        }
                     }
                     else if ( strcmp( parameter, "STOP" ) == 0 )
                     {
                        cmd.controlFrame.setDataLength( 1 );
                        cmd.controlFrame.setCommand( Dimmer::Command::STOP );
                     }
                     else
                     {
                        isInvalid = true;
                     }
                  }
                  else
                  {
                     WARN_3( FSTR( "LoxoneGateway::evData() command decoder for classId " ), parameter, FSTR( " not implemented" ) );
                     return false;
                  }

                  if ( isInvalid )
                  {
                     WARN_2( FSTR( "LoxoneGateway::evData() invalid command: " ), parameter );
                     return false;
                  }
                  break;
               }

               default:
               {
                  WARN_4( FSTR( "LoxoneGateway::evData() unexpected data: " ), parameter, FSTR( " at idx: " ), i );
                  break;
               }
            }
            parameter = strtok( NULL, MSG_TAG );
            i++;
         }

         notifyMessageReceived( &cmd.controlFrame );
      }
   }
   else
   {
      return Gateway::notifyEvent( event );
   }
   return true;
}

IStream::Status LoxoneGateway::genericCommand( IoStream::Command command, void* buffer )
{
   return udpStream->genericCommand( command, buffer );
}

IStream::Status LoxoneGateway::read( void* pData, uint16_t length, EventDrivenUnit* user )
{
   // receiving data will be notified and processed by event
   return IStream::NOT_SUPPORTED;
}

IStream::Status LoxoneGateway::write( void* pData, uint16_t length, EventDrivenUnit* user )
{
   // ToDo: convert into LOXONE format before sending data
   HBCP* msg = reinterpret_cast<HBCP*>( pData );
   char loxoneMsg[MAX_MSG_SIZE];
   char instanceIdString[8];
   bool isUnknow = false;

   HBCP::Object* sender = msg->getControlFrame()->getSenderId();
   uint8_t event = msg->getControlFrame()->getData()[0];
   utoa( sender->getInstanceId(), instanceIdString, 10 );
   ultoa( sender->getDeviceId(), loxoneMsg, 10 );
   strcat( loxoneMsg, MSG_TAG );

   switch ( sender->getClassId() )
   {
      case ClassId::BUTTON:
      {
         strcat( loxoneMsg, "KEY" );
         strcat( loxoneMsg, MSG_TAG );
         strcat( loxoneMsg, instanceIdString );
         strcat( loxoneMsg, MSG_TAG );

         switch ( event )
         {
            case Button::Response::EVENT_FREE:
            {
               strcat( loxoneMsg, "FREE" );
               break;
            }
            case Button::Response::EVENT_COVERED:
            {
               strcat( loxoneMsg, "COVERED" );
               break;
            }
            case Button::Response::EVENT_CLICKED:
            {
               strcat( loxoneMsg, "CLICKED" );
               break;
            }
            case Button::Response::EVENT_DOUBLE_CLICKED:
            {
               strcat( loxoneMsg, "DOUBLE_CLICKED" );
               break;
            }
            case Button::Response::EVENT_HOLD_START:
            {
               strcat( loxoneMsg, "HOLD_START" );
               break;
            }
            case Button::Response::EVENT_HOLD_END:
            {
               strcat( loxoneMsg, "HOLD_END" );
               break;
            }

            default:
            {
               isUnknow = true;
               break;
            }
         }
         break;
      }

      case ClassId::TEMPERATURE:
      {
         strcat( loxoneMsg, "TEMP" );
         strcat( loxoneMsg, MSG_TAG );
         strcat( loxoneMsg, instanceIdString );
         strcat( loxoneMsg, MSG_TAG );
         switch ( event )
         {
            case BaseSensorUnit::Response::STATUS:
            {
               strcat( loxoneMsg, "STATUS" );
               strcat( loxoneMsg, MSG_TAG );

               int8_t value = (int8_t)msg->getControlFrame()->getData()[1];
               uint8_t centiValue = msg->getControlFrame()->getData()[2];
               char temp[8];
               itoa( value, temp, 10 );
               strcat( loxoneMsg, temp );
               strcat( loxoneMsg, "," );
               if ( centiValue < 10 )
               {
                  strcat( loxoneMsg, "0" );
               }
               utoa( centiValue, temp, 10 );
               strcat( loxoneMsg, temp );
               break;
            }
            default:
            {
               isUnknow = true;
               break;
            }
         }
         break;
      }

      case ClassId::HUMIDITY:
      {
         strcat( loxoneMsg, "HUMIDITY" );
         strcat( loxoneMsg, MSG_TAG );
         strcat( loxoneMsg, instanceIdString );
         strcat( loxoneMsg, MSG_TAG );
         switch ( event )
         {
            case BaseSensorUnit::Response::STATUS:
            {
               strcat( loxoneMsg, "STATUS" );
               strcat( loxoneMsg, MSG_TAG );

               char temp[8];
               itoa( msg->getControlFrame()->getData()[1], temp, 10 );
               strcat( loxoneMsg, temp );
               break;
            }
            default:
            {
               isUnknow = true;
               break;
            }
         }
         break;
      }

      case ClassId::LED:
      {
         strcat( loxoneMsg, "LED" );
         strcat( loxoneMsg, MSG_TAG );
         strcat( loxoneMsg, instanceIdString );
         strcat( loxoneMsg, MSG_TAG );
         Led::Response* ledEvent = (Led::Response*)msg;
         switch ( event )
         {
            case Led::Response::EVENT_ON:
            case Led::Response::EVENT_OFF:
            {
               strcat( loxoneMsg, "STATUS" );
               strcat( loxoneMsg, MSG_TAG );

               uint8_t status = 0;
               if ( event == Led::Response::EVENT_ON )
               {
                  status = ledEvent->getParameter().status;
               }
               char temp[8];
               itoa( status, temp, 10 );
               strcat( loxoneMsg, temp );
               break;
            }
            default:
            {
               isUnknow = true;
               break;
            }
         }
         break;
      }

      case ClassId::DIGITAL_OUTPUT:
      {
         strcat( loxoneMsg, "OUT" );
         strcat( loxoneMsg, MSG_TAG );
         strcat( loxoneMsg, instanceIdString );
         strcat( loxoneMsg, MSG_TAG );

         switch ( event )
         {
            case PortPinUnit::Response::EVENT_ON:
            case PortPinUnit::Response::EVENT_OFF:
            {
               strcat( loxoneMsg, "STATUS" );
               strcat( loxoneMsg, MSG_TAG );

               uint8_t status = 0;
               if ( event == PortPinUnit::Response::EVENT_ON )
               {
                  status = 1;
               }
               char temp[8];
               itoa( status, temp, 10 );
               strcat( loxoneMsg, temp );
               break;
            }
            default:
            {
               isUnknow = true;
               break;
            }
         }
         break;
      }

      case ClassId::DIMMER:
      {
         strcat( loxoneMsg, "DIM" );
         strcat( loxoneMsg, MSG_TAG );
         strcat( loxoneMsg, instanceIdString );
         strcat( loxoneMsg, MSG_TAG );
         Dimmer::Response* dimmerEvent = (Dimmer::Response*)msg;
         switch ( event )
         {
            case Dimmer::Response::EVENT_ON:
            case Dimmer::Response::EVENT_OFF:
            {
               strcat( loxoneMsg, "STATUS" );
               strcat( loxoneMsg, MSG_TAG );

               uint8_t status = 0;
               if ( event == Dimmer::Response::EVENT_ON )
               {
                  status = dimmerEvent->getParameter().status;
               }
               char temp[8];
               itoa( status, temp, 10 );
               strcat( loxoneMsg, temp );
               break;
            }
            default:
            {
               isUnknow = true;
               break;
            }
         }
         break;
      }

      default:
      {
         isUnknow = true;
         break;
      }
   }
   if ( isUnknow )
   {
      strcat( loxoneMsg, "UNKNOWN" );
   }

   DEBUG_H1( FSTR( "->sending Loxone message:" ) );
   DEBUG_M1( loxoneMsg );

   if ( isUnknow )
   {
      // do not send UNKNOWN / NOT_SUPPORTED messages
      return IStream::SUCCESS;
   }
   return udpStream->write( loxoneMsg, strlen( loxoneMsg ), user );
}
