/*
 * HbcInterface.h
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#ifndef Systems_HomeAutomationInterface_H
#define Systems_HomeAutomationInterface_H

#include "HbcConfiguration.h"
#include <MemoryManager.h>
#include <Security/ModuleId.h>
#include <IResponse.h>


class HbcInterface
{
   public:

      class Command;

      class ErrorCode;

      class Response;

      static const uint32_t DATA_SECTION_MASK = 0xC0000000;

      class Command
      {
         public:

            enum Commands
            {
               GENERATE_RANDOM_DEVICE_ID = HBCP::COMMANDS_START,
               RESET,
               GET_MODULE_ID,
               GET_REMOTE_OBJECTS,
               GET_UNUSED_MEMORY,
               GET_CONFIGURATION,
               SET_CONFIGURATION,
               READ_MEMORY,
               WRITE_MEMORY,
               WRITE_RULES,
               READ_RULES,
               SET_RULE_STATE,
               GET_RULE_STATE,
               TRIGGER_RULE_ELEMENT,
               SET_UNIT_GROUP_STATE,
               SET_SUN_TIMES,
               SET_SYSTEM_VARIABLE,
               GET_SYSTEM_VARIABLE,

               SET_DEBUG_OPTIONS = HBCP::COMMANDS_END - 3,
               SET_TIME = HBCP::COMMANDS_END - 2,
               GET_TIME = HBCP::COMMANDS_END - 1,
               PING = HBCP::COMMANDS_END
            };

            struct GetModuleId
            {
               uint8_t index;
            };

            struct ReadMemory
            {
               uint32_t address;
               uint16_t length;
            };

            struct WriteMemory
            {
               uint32_t address;
               uint8_t data[2];
            };

            struct ReadRules
            {
               uint16_t offset;
               uint16_t length;
               uint8_t data[2];
            };

            struct WriteRules
            {
               uint16_t offset;
               uint8_t data[2];
            };

            struct SetRuleState
            {
               uint8_t index;
               uint8_t state;
            };

            struct TriggerRuleElement
            {
               uint8_t idxRule;
               uint8_t idxElement;
            };

            struct SetUnitGroupState
            {
               uint8_t index;
               uint8_t member;
               uint8_t state;
               uint8_t triggerBits;
            };

            struct SetSystemVariable
            {
               uint8_t type;
               uint8_t index;
               uint16_t value;
            };

            struct GetSystemVariable
            {
               uint8_t type;
               uint8_t index;
            };

            struct SunTimes
            {
               uint16_t sunRise;
               uint16_t sunSet;
            };

            union Parameter
            {
               GetModuleId getModuleId;
               HbcConfiguration setConfiguration;
               ReadMemory readMemory;
               WriteMemory writeMemory;
               WriteRules writeRules;
               ReadRules readRules;
               uint16_t time;
               uint8_t index;
               SetRuleState setRuleState;
               TriggerRuleElement triggerRuleElement;
               SetUnitGroupState setUnitGroupState;
               SetSystemVariable setSystemVariable;
               GetSystemVariable getSystemVariable;
               uint8_t debugOptions;
               SunTimes sunTimes;
            };

            ////    Operations    ////

            inline Parameter& getParameter()
            {
               return parameter;
            }

            ////    Additional operations    ////

            inline uint8_t getCommand() const
            {
               return command;
            }

            inline void setCommand( uint8_t p_command )
            {
               command = p_command;
            }

            inline void setParameter( Parameter p_parameter )
            {
               parameter = p_parameter;
            }

            ////    Attributes    ////

            uint8_t command;

            Parameter parameter;
      };

      class Response : public IResponse
      {
         public:

            enum Responses
            {
               MODULE_ID = HBCP::RESULTS_START,
               REMOTE_OBJECTS,
               UNUSED_MEMORY,
               CONFIGURATION,
               MEMORY_DATA,
               MEMORY_STATUS,
               RULES_DATA,
               RULE_STATE,
               RULE_TRIGGERED,
               SYSTEM_VARIABLE,

               TIME_DIFFERENCE = HBCP::RESULTS_END - 2,
               TIME,
               PONG,
               EVENT_TIME = HBCP::EVENTS_START,
               EVENT_NEW_DEVICE_ID,
               EVENT_STARTED,
               EVENT_GROUP_ON,
               EVENT_GROUP_UNDEFINED,
               EVENT_GROUP_OFF,
            };

            struct Configuration
            {
               HbcConfiguration hwConfiguration;
               uint16_t dataBlockSize;
               uint8_t fcke;
            };

            struct MemoryData
            {
               uint32_t address;
               uint8_t data[HBCP::MAX_DATA_SIZE];
            };

            struct RuleData
            {
               uint16_t offset;
               uint8_t data[2];
            };

            struct RuleState
            {
               uint8_t index;
               uint8_t state;
            };

            struct SystemVariable
            {
               uint8_t type;
               uint8_t index;
               uint16_t value;
            };

            struct UnusedMemory
            {
               uint16_t freeStack;
               uint16_t freeHeap;
            };

            struct UnitGroupEvent
            {
               uint8_t index;
            };

            union Parameter
            {
               ModuleId moduleId;
               Configuration configuration;
               uint8_t status;
               MemoryData memoryData;
               uint16_t remoteObjects[2];
               UnusedMemory unusedMemory;
               RuleData rulesData;
               uint16_t deviceId;
               uint16_t time;
               RuleState ruleState;
               UnitGroupEvent unitGroupEvent;
               SystemVariable systemVariable;
               int8_t timeDifference;
            };

            ////    Constructors and destructors    ////

            Response( uint16_t _id );

            ////    Operations    ////

            inline Parameter& getParameter()
            {
               return *reinterpret_cast<Parameter*>( IResponse::getParameter() );
            }

            inline HbcConfiguration& setConfiguration( uint8_t fcke );

            inline void setDeviceId( uint16_t _id );

            inline void setMemoryStatus( uint8_t status );

            inline ModuleId* setModuleId();

            inline void setPong();

            inline uint8_t* setReadMemory( const uint32_t& address, uint16_t size );

            inline uint8_t* setReadRules( uint16_t offset, uint16_t length );

            inline void setRuleState( uint8_t index, uint8_t state );

            inline void setStarted( uint8_t sources );

            inline void setTime( uint16_t value );

            inline void setTimeDifference( int8_t value );

            inline void setUnitGroupEvent( uint8_t event, uint8_t index );

            inline void setSystemVariable( uint8_t type, uint8_t index, uint16_t value );

            inline void setUnusedMemory();

            ////    Attributes    ////

         private:

            Parameter params;
      };

      class ErrorCode
      {
         public:

            enum Errors
            {
               OKAY,
               MODULE_NOT_EXISTS,
               MEMORY_WRITE_FAILED,
               INVALID_RULE_TABLE,
               SYNTAX_ERROR,
               NULL_POINTER,
               LOW_VOLTAGE,
               WATCHDOG,
               INVALID_FW_LOADED,
               MSG_QUEUE_OVERRUN,
               MAX_OBJECTS_REACHED,

               CHECKSUM_ERROR = 16,
               CHECKSUM_ERROR_CONF_FLASH,
               CHECKSUM_ERROR_CONF_EEPROM,
            };
      };
};

inline HbcConfiguration& HbcInterface::Response::setConfiguration( uint8_t fcke )
{
   controlFrame.setDataLength( sizeof( getResponse() ) + sizeof( getParameter().configuration ) );
   setResponse( CONFIGURATION );
   getParameter().configuration.dataBlockSize = HBCP::MAX_DATA_SIZE;
   getParameter().configuration.fcke = fcke;
   return getParameter().configuration.hwConfiguration;
}

inline void HbcInterface::Response::setDeviceId( uint16_t _id )
{
   controlFrame.setDataLength( sizeof( getResponse() ) + sizeof( getParameter().deviceId ) );
   setResponse( EVENT_NEW_DEVICE_ID );
   getParameter().deviceId = _id;
}

inline void HbcInterface::Response::setMemoryStatus( uint8_t status )
{
   controlFrame.setDataLength( sizeof( getResponse() ) + sizeof( getParameter().status ) );
   setResponse( MEMORY_STATUS );
   getParameter().status = status;
}

inline ModuleId* HbcInterface::Response::setModuleId()
{
   controlFrame.setDataLength( sizeof( getResponse() ) + sizeof( getParameter().moduleId ) );
   setResponse( MODULE_ID );
   return &getParameter().moduleId;
}

inline void HbcInterface::Response::setPong()
{
   controlFrame.setDataLength( sizeof( getResponse() ) );
   setResponse( PONG );
}

inline uint8_t* HbcInterface::Response::setReadMemory( const uint32_t& address, uint16_t size )
{
   controlFrame.setDataLength( sizeof( getResponse() ) + sizeof( getParameter().memoryData.address ) + size );
   setResponse( MEMORY_DATA );
   getParameter().memoryData.address = address;
   return getParameter().memoryData.data;
}

inline uint8_t* HbcInterface::Response::setReadRules( uint16_t offset, uint16_t length )
{
   controlFrame.setDataLength( sizeof( getResponse() ) + sizeof( getParameter().rulesData.offset ) + length );
   setResponse( RULES_DATA );
   getParameter().rulesData.offset = offset;
   return getParameter().rulesData.data;
}

inline void HbcInterface::Response::setRuleState( uint8_t index, uint8_t state )
{
   controlFrame.setDataLength( sizeof( getResponse() ) + sizeof( getParameter().ruleState ) );
   setResponse( RULE_STATE );
   getParameter().ruleState.index = index;
   getParameter().ruleState.state = state;
}

inline void HbcInterface::Response::setStarted( uint8_t sources )
{
   controlFrame.setDataLength( sizeof( getResponse() ) + sizeof( getParameter().status ) );
   setResponse( EVENT_STARTED );
   getParameter().status = sources;
}

inline void HbcInterface::Response::setTime( uint16_t value )
{
   controlFrame.setDataLength( sizeof( getResponse() ) + sizeof( getParameter().time ) );
   setResponse( TIME );
   getParameter().time = value;
}

inline void HbcInterface::Response::setTimeDifference( int8_t value )
{
   controlFrame.setDataLength( sizeof( getResponse() ) + sizeof( getParameter().timeDifference ) );
   setResponse( TIME_DIFFERENCE );
   getParameter().timeDifference = value;
}

inline void HbcInterface::Response::setUnitGroupEvent( uint8_t event, uint8_t index )
{
   controlFrame.setDataLength( sizeof( getResponse() ) + sizeof( getParameter().unitGroupEvent ) );
   setResponse( event );
   getParameter().unitGroupEvent.index = index;
}

inline void HbcInterface::Response::setSystemVariable( uint8_t type, uint8_t index, uint16_t value )
{
   controlFrame.setDataLength( sizeof( getResponse() ) + sizeof( getParameter().systemVariable ) );
   setResponse( SYSTEM_VARIABLE );
   getParameter().systemVariable.type = type;
   getParameter().systemVariable.index = index;
   getParameter().systemVariable.value = value;
}

inline void HbcInterface::Response::setUnusedMemory()
{
   controlFrame.setDataLength( sizeof( getResponse() ) + sizeof( getParameter().unusedMemory ) );
   setResponse( UNUSED_MEMORY );
   getUnusedMemory( &getParameter().unusedMemory.freeStack, &getParameter().unusedMemory.freeHeap );
}

#endif
