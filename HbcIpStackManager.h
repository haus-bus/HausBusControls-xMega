/*
 * HbcIpStackManager.h
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#ifndef SwUnits_HacfIpStackManager_H
#define SwUnits_HacfIpStackManager_H

#include "HbcUnits.h"
#include <ConfigurationManager.h>
#include <Protocols/IpStack/IpStackManager.h>
#include <Protocols/Ethernet/MAC.h>
class ArpHeader;

class ArpManager;

class CriticalSection;

class Dhcp;

class Enc28j60;

class Event;

class HBCP;

class IP;

class IcmpHeader;

class MAC;

class ModBusSlave;

class Scheduler;

class TcpConnection;

class UdpConnection;

class UdpHeader;

class evMessage;

class HbcIpStackManager : public IpStackManager
{
   public:

      class Configuration
      {
         public:

            static const uint8_t DEFAULT_OPTIONS = 1;

            struct Options
            {
               bool udpPort9Only : 1;

               bool modBusTcp : 1;

               bool dhcp : 1;
            };

            union Option
            {
               uint8_t mask;
               Options bit;
            };

            ////    Attributes    ////

            uint32_t ip;

            Option option;

            uint16_t port;

            ////    Operations    ////

            static inline Configuration getDefault()
            {
               Configuration defaultConfiguration =
               {
                  .ip = IP::defaultIp.address,
                  .option = { DEFAULT_OPTIONS },
                  .port = 0,
               };
               return defaultConfiguration;
            }

            inline void checkAndCorrect()
            {
            }
      };

      class EepromConfiguration : public ConfigurationManager::EepromConfigurationTmpl<Configuration>
      {
         public:

            uint32_tx ip;

            XEeprom<Configuration::Option> option;

            uint16_tx port;

            inline Configuration::Options getOptions() const
            {
               return option.operator*().bit;
            }
      };

      class Command
      {
         public:

            enum Commands
            {
               GET_CONFIGURATION = HBCP::COMMANDS_START,
               SET_CONFIGURATION,
               WAKE_UP_DEVICE,
               GET_CURRENT_IP
            };

            union Parameter
            {
               MAC mac;
               Configuration setConfiguration;
            };

            ////    Operations    ////

            uint8_t getCommand() const;

            inline Parameter& getParameter()
            {
               return parameter;
            }

            void setCommand( uint8_t p_command );

            void setParameter( const Parameter& p_parameter );

            ////    Attributes    ////

            uint8_t command;

            Parameter parameter;
      };

      class Response : public IResponse
      {
         public:

            enum Responses
            {
               CONFIGURATION = HBCP::RESULTS_START,
               CURRENT_IP
            };

            union Parameter
            {
               Configuration configuration;
               uint32_t ip;
            };

            ////    Constructors and destructors    ////

            inline Response( uint16_t id ) :
               IResponse( id )
            {
            }

            inline Response( uint16_t id, const HBCP& message ) :
               IResponse( id, message )
            {
            }

            ////    Operations    ////

            inline Parameter& getParameter()
            {
               return *reinterpret_cast<Parameter*>( IResponse::getParameter() );
            }

            Parameter& setConfiguration();

            void setCurrentIp();

            ////    Attributes    ////

         private:

            Parameter params;
      };

      ////    Constructors and destructors    ////

      HbcIpStackManager( Enc28j60& _stream );

      ////    Operations    ////

      virtual bool notifyEvent( const Event& event );

      inline void setConfiguration( EepromConfiguration* _config )
      {
         configuration = _config;
      }

      void wakeUpDevice( const MAC& mac );

   protected:

      bool handleRequest( HBCP* message );

      ////    Relations and components    ////

      EepromConfiguration* configuration;
};

#endif
