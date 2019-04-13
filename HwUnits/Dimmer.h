/*
 * Dimmer.h
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#ifndef HwUnits_Dimmer_H
#define HwUnits_Dimmer_H

#include "HbcUnits.h"
#include "HwUnitBoards/IDimmerHw.h"
#include <ConfigurationManager.h>


class CriticalSection;

class Event;

class HBCP;

class IDimmerHw;

class Scheduler;

class evMessage;

class Dimmer : public Reactive
{
   public:

      enum SubStates
      {
         NO_COMMAND,
         DIMMING
      };

      enum Direction
      {
         TO_DARK = -1,
         TOGGLE = 0,
         TO_LIGHT = 1
      };

      static const uint16_t TIME_BASE = SystemTime::S;

      struct Configuration
      {
         static const uint8_t TIME_FACTOR = 50;

         enum Mode
         {
            DIMM_CR, // CR
            DIMM_L, // L
            SWITCH,
            MAX_MODE
         };

         static const Mode DEFAULT_MODE = DIMM_CR;
         static const uint8_t DEFAULT_FADING_TIME = 600 / TIME_FACTOR;
         static const uint8_t DEFAULT_DIMMING_TIME = 3000 / TIME_FACTOR;
         static const uint8_t DEFAULT_DIMMING_RANGE_START = 0;
         static const uint8_t DEFAULT_DIMMING_RANGE_END = 100;

         static const uint8_t MAX_FADING_TIME = 200;
         static const uint8_t MAX_DIMMING_TIME = 200;
         static const uint8_t MAX_DIMMING_RANGE_START = 100;
         static const uint8_t MAX_DIMMING_RANGE_END = 100;

         ////    Attributes    ////

         Mode mode;

         uint8_t fadingTime;

         uint8_t dimmingTime;

         uint8_t dimmingRangeStart;

         uint8_t dimmingRangeEnd;

         ////    Functions    ////

         static inline Configuration getDefault()
         {
            Configuration defaultConfiguration =
            {
               .mode = DEFAULT_MODE,
               .fadingTime = DEFAULT_FADING_TIME,
               .dimmingTime = DEFAULT_DIMMING_TIME,
               .dimmingRangeStart = DEFAULT_DIMMING_RANGE_START,
               .dimmingRangeEnd = DEFAULT_DIMMING_RANGE_END
            };
            return defaultConfiguration;
         }

         inline void checkAndCorrect()
         {
            if ( dimmingRangeStart > MAX_DIMMING_RANGE_START )
            {
               dimmingRangeStart = MAX_DIMMING_RANGE_START;
            }
            if ( dimmingRangeEnd > MAX_DIMMING_RANGE_END )
            {
               dimmingRangeEnd = MAX_DIMMING_RANGE_END;
            }
            if ( dimmingTime > MAX_DIMMING_TIME )
            {
               dimmingTime = MAX_DIMMING_TIME;
            }
            if ( fadingTime > MAX_FADING_TIME )
            {
               fadingTime = MAX_FADING_TIME;
            }
            if ( dimmingRangeStart >= dimmingRangeEnd )
            {
               dimmingRangeStart = DEFAULT_DIMMING_RANGE_START;
               dimmingRangeEnd = DEFAULT_DIMMING_RANGE_END;
            }
         }
      };

      class EepromConfiguration : public ConfigurationManager::EepromConfigurationTmpl<Configuration>
      {
         public:
            XEeprom<Configuration::Mode> mode;

            uint8_tx fadingTime;

            uint8_tx dimmingTime;

            uint8_tx dimmingRangeStart;

            uint8_tx dimmingRangeEnd;

            uint16_tx reserve;
      };

      class Command
      {
         public:

            enum Commands
            {
               GET_CONFIGURATION = HBCP::COMMANDS_START,
               SET_CONFIGURATION,
               SET_BRIGHTNESS,
               START,
               STOP,
               GET_STATUS,
            };

            struct SetBrightness
            {
               uint8_t brightness;
               uint16_t duration;
            };

            struct Start
            {
               int8_t direction;
            };

            union Parameter
            {
               SetBrightness setBrightness;
               Configuration setConfiguration;
               Start start;
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
               STATUS,

               EVENT_OFF = HBCP::EVENTS_START,
               EVENT_ON,
               EVENT_START,

               EVENT_ERROR = HBCP::EVENTS_END
            };

            union Parameter
            {
               Configuration configuration;
               uint8_t brightness;
               int8_t direction;
               uint8_t status;
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

            void setBrightness( uint16_t brightness );

            Parameter& setConfiguration();

            void setDirection( uint8_t direction );

            void setStatus( uint16_t brightness );

            ////    Attributes    ////

         private:

            Parameter params;

      };

      ////    Constructors and destructors    ////

      Dimmer( uint8_t _id, IDimmerHw* _hardware );

      ////    Operations    ////

      virtual bool notifyEvent( const Event& event );

      inline void* operator new( size_t size );

      void run();

   private:

      void cmdSetBrightness( Command::SetBrightness& parameter, uint8_t changeTime
                                = 0 );

      void cmdStart( const Command::Start& parameter );

      void configureHw();

      inline uint16_t getStepWidth( uint8_t time = 0 );

      inline int16_t getDimmingRangeEnd() const
      {
         return configuration->dimmingRangeEnd * 10;
      }

      inline int16_t getDimmingRangeStart() const
      {
         return configuration->dimmingRangeStart * 10;
      }

      bool handleRequest( HBCP* message );

      void handleRunningState();

      void handleSuspend();

      void notifyError( uint8_t errorCode );

      ////    Additional operations    ////

   public:

      IDimmerHw* getHardware() const;

      void setHardware( IDimmerHw* p_IDimmerHw );

   private:

      inline uint16_t getBrightness() const
      {
         return brightness;
      }

      inline void setBrightness( uint16_t p_brightness )
      {
         brightness = p_brightness;
      }

      inline static const uint8_t getDebugLevel()
      {
         return debugLevel;
      }

      inline int16_t getDirection() const
      {
         return direction;
      }

      inline void setDirection( int16_t p_direction )
      {
         direction = p_direction;
      }

      inline uint16_t getDuration() const
      {
         return duration;
      }

      inline void setDuration( uint16_t p_duration )
      {
         duration = p_duration;
      }

      inline SubStates getSubState() const
      {
         return subState;
      }

      inline void setSubState( SubStates p_subState )
      {
         subState = p_subState;
      }

      inline uint16_t getTargetBrightness() const
      {
         return targetBrightness;
      }

      inline void setTargetBrightness( uint16_t p_targetBrightness )
      {
         targetBrightness = p_targetBrightness;
      }

      inline void setConfiguration( EepromConfiguration* _config )
      {
         configuration = _config;
      }

      ////    Attributes    ////

      int16_t brightness;

      static const uint8_t debugLevel;

      int16_t direction;

      uint16_t duration;

      SubStates subState;

      int16_t targetBrightness;

      ////    Relations and components    ////

   protected:

      EepromConfiguration* configuration;

      IDimmerHw* hardware;

};

inline void* Dimmer::operator new( size_t size )
{
   return allocOnce( size );
}

inline uint16_t Dimmer::getStepWidth( uint8_t time )
{
   if ( time == 0 )
   {
      time = configuration->fadingTime;
   }
   if ( time == 0 )
   {
      time = 1;
   }

   return 10000 / Configuration::TIME_FACTOR / time;
}

#endif
