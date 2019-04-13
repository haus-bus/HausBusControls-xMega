/*
 * HbcIpStackManager.cpp
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#include "HbcIpStackManager.h"
#include "ModBusSlave.h"
#include <Protocols/Ethernet/ArpHeader.h>
#include <Protocols/IpStack/ArpManager.h>
#include <Protocols/IpStack/Dhcp.h>
#include <Enc28j60.h>
#include <Protocols/Ethernet/IcmpHeader.h>
#include <Protocols/Ethernet/IP.h>
#include <Protocols/Ethernet/MAC.h>
#include <Protocols/IpStack/TcpConnection.h>
#include <Protocols/IpStack/UdpConnection.h>
#include <Protocols/Ethernet/UdpHeader.h>
#include <ErrorMessage.h>

uint8_t HbcIpStackManager::Command::getCommand() const
{
   return command;
}

void HbcIpStackManager::Command::setCommand( uint8_t p_command )
{
   command = p_command;
}

void HbcIpStackManager::Command::setParameter( const HbcIpStackManager::Command::Parameter& p_parameter )
{
   parameter = p_parameter;
}

HbcIpStackManager::Response::Parameter& HbcIpStackManager::Response::setConfiguration()
{
   controlFrame.setDataLength( sizeof( getResponse() ) + sizeof( getParameter().configuration ) );
   setResponse( CONFIGURATION );
   return getParameter();
}

void HbcIpStackManager::Response::setCurrentIp()
{
   controlFrame.setDataLength( sizeof( getResponse() ) + sizeof( getParameter().ip ) );
   setResponse( CURRENT_IP );
   getParameter().ip = IP::local.getAddress();
}

HbcIpStackManager::HbcIpStackManager( Enc28j60& _stream )
{
   IpConnection::stream = &_stream;
   setId( ( ClassId::ETHERNET << 8 ) | 1 );
   setConfiguration( ConfigurationManager::getConfiguration<EepromConfiguration>( id ) );
}

bool HbcIpStackManager::notifyEvent( const Event& event )
{
   if ( event.isEvMessage() )
   {
      return handleRequest( event.isEvMessage()->getMessage() );
   }
   else if ( event.isEvWakeup() && inStartUp() )
   {
      if ( configuration )
      {
         SET_STATE_L1( RUNNING );
      }
      else
      {
         terminate();
         ErrorMessage::notifyOutOfMemory( id );
         return true;
      }

      IP::local.setAddress( configuration->ip );
      Configuration::Options options = configuration->getOptions();

      if ( options.udpPort9Only )
      {
         IpConnection::stream->setUdpPort9Filter();
      }
      else
      {
         if ( IP::local.isBroadcast() || !IP::local.isValid() || options.dhcp )
         {
            new Dhcp();
         }
         if ( options.modBusTcp )
         {
            new ModBusSlave();
         }
         if ( configuration->port )
         {
            new Gateway( UdpConnection::connect( IP::broadcast(), configuration->port, configuration->port, NULL ), Gateway::UDP );
         }
      }
   }

   return IpStackManager::notifyEvent( event );
}

void HbcIpStackManager::wakeUpDevice( const MAC& mac )
{
}

bool HbcIpStackManager::handleRequest( HBCP* message )
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
   else if ( cf.isCommand( Command::WAKE_UP_DEVICE ) )
   {
      DEBUG_H1( FSTR( ".wakeUpDevice()" ) );
      wakeUpDevice( data->parameter.mac );
   }
   else if ( cf.isCommand( Command::GET_CURRENT_IP ) )
   {
      DEBUG_H1( FSTR( ".getCurrentIp()" ) );
      Response response( getId(), *message );
      response.setCurrentIp();
      response.queue( getObject( message->header.getSenderId() ) );
   }
   else
   {
      return false;
   }

   return consumed;
}

