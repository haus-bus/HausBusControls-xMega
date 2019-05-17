/*
 * LoxoneUdp.h
 *
 *  Created on: 15.05.2019
 *      Author: Viktor Pankraz
 */

#ifndef SwFramework_Protocols_LoxoneUdp_H
#define SwFramework_Protocols_LoxoneUdp_H

#include "Gateway.h"

#include <IoStream.h>


class LoxoneGateway : public IoStream,
                             Gateway
{
   public:

      static const uint8_t MAX_MSG_SIZE = 128;
      static const char MSG_TAG[];

      enum LoxoneMsgIndex
      {
         IDX_DEVICE_ID,
         IDX_CLASS_ID,
         IDX_INSTANCE_ID,
         IDX_COMMAND,
      };

      ////    Constructors and destructors    ////

      LoxoneGateway( IoStream* _updStream ) : Gateway( this, LOXONE ), udpStream( _updStream )
      {

      }

      ////    Operations    ////

      IStream::Status genericCommand( IoStream::Command command, void* buffer );

      IStream::Status read( void* pData, uint16_t length, EventDrivenUnit* user = 0 );

      IStream::Status write( void* pData, uint16_t length, EventDrivenUnit* user = 0 );

      virtual bool notifyEvent( const Event& event );

      inline void* operator new( size_t size )
      {
         return allocOnce( size );
      }

      inline uint16_t getId() const
      {
         return Gateway::getId();
      }

      ////    Additional operations    ////

   protected:

      IoStream* udpStream;

      ////    Attributes    ////

      static const uint8_t debugLevel;
};


#endif
