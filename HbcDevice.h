/*
 * HbcDevice.h
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#ifndef Systems_HomeAutomation_H
#define Systems_HomeAutomation_H

#include "HbcInterface.h"
#include "HbcConfiguration.h"
#include "HwUnitBoards/HbcDeviceHw.h"

#include <Reactive.h>
#include <Time/SystemTime.h>
#include <IResponse.h>

#include <Time/Calender.h>

class CriticalSection;

class DebugOptions;

class Event;

class Gateway;

class HBCP;

class PersistentRules;

class RuleEngine;

class Scheduler;

class WeekTime;

class evMessage;

class HbcDevice : public Reactive
{
   public:

      class LogicalUnitGroup;

      class MyEvent;

      static const uint16_t TIME_TO_RESET = 100;
      static const uint8_t MAX_TIME_DIFFERENCE = 8;

      class LogicalUnitGroup
      {
         public:

            static const uint8_t MAX_GROUPS = 16;

            ////    Operations    ////

            static uint8_t setState( const HbcInterface::Command::SetUnitGroupState& params );

            ////    Attributes    ////

            static uint16_t state[MAX_GROUPS];
      };

      class MyEvent : public IResponse
      {
         public:

            enum Events
            {
               EVENT_TIME = HBCP::EVENTS_START,
               EVENT_NEW_DEVICE_ID,
               EVENT_STARTED,
               EVENT_GROUP_ON,
               EVENT_GROUP_UNDEFINED,
               EVENT_GROUP_OFF,
               EVENT_DAY,
               EVENT_NIGHT
            };

            ////    Constructors and destructors    ////

            inline MyEvent() : IResponse( HBCP::SYSTEM_ID )
            {
               controlFrame.receiverId.setId( ( (uint32_t) HBCP::deviceId << 16 ) | HBCP::SYSTEM_ID );
               controlFrame.setDataLength( 1 );
            }

            ////    Operations    ////

            void setTimeEvent();
      };

      ////    Constructors and destructors    ////

      inline HbcDevice() : errorEvent( HBCP::SYSTEM_ID )
      {
         setId( ( ClassId::SYSTEM << 8 ) | HBCP::SYSTEM_ID );
         HbcConfiguration& conf = HbcConfiguration::instance();
         HBCP::deviceId = conf.getDeviceId();
         Calender::minuteListener = this;
         // give some additional time to get e.g. a valid IP address from DHCP Server
         setGlobalSleepDelay( conf.getStartupDelay() << 3 );
      }

      ////    Operations    ////

      virtual bool notifyEvent( const Event& event );

      void run();

   protected:

      void checkPersistentRules();

      void cmdGetRemoteObjects( HbcInterface::Response& response );

      void cmdReadMemory( HbcInterface::Command::ReadMemory& parameter,
                          HbcInterface::Response& response );

      void cmdWriteRules( HbcInterface::Command::WriteRules& parameter,
                          uint16_t dataLength,
                          HbcInterface::Response& response );

      bool handleRequest( HBCP* message );

   private:

      uint16_t getMinuteTicks() const;

      ////    Additional operations    ////

   public:

      HbcInterface::Response* getErrorEvent() const;

   protected:

      inline static const uint8_t getDebugLevel()
      {
         return debugLevel;
      }

   private:

      ////    Attributes    ////

   protected:

      static const uint8_t debugLevel;

   private:

      static Timestamp lastMemoryReportTime;

      ////    Relations and components    ////

   protected:

      HbcInterface::Response errorEvent;

};


#endif
