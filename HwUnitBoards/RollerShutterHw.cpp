/*
 * RollerShutterHw.cpp
 *
 * Created: 18.06.2014 14:12:55
 * Author: viktor.pankraz
 */


#include "RollerShutterHw.h"
#include <Tracing/Logger.h>
#include <Time/SystemTime.h>


bool RollerShutterHw::isDirectionToClose()
{
   if ( independent )
   {
      return direction == TO_CLOSE;
   }
   return directionOutput.isSet();
}

bool RollerShutterHw::isDirectionToOpen()
{
   if ( independent )
   {
      return direction == TO_OPEN;
   }
   return !directionOutput.isSet();
}

void RollerShutterHw::off()
{
   powerOutput.clear();

   if ( independent )
   {
      directionOutput.clear();
      return;
   }
   SystemTime::waitMs( POWER_SWITCH_DELAY );

   if ( directionOutput.isInverted() )
   {
      directionOutput.set();
   }
   else
   {
      directionOutput.clear();
   }
}

void RollerShutterHw::on()
{
   if ( independent )
   {
      if ( direction ^ inverted )
      {
         directionOutput.set();
      }
      else
      {
         powerOutput.set();
      }
      return;
   }
   powerOutput.set();
   SystemTime::waitMs( POWER_SWITCH_DELAY );
}

void RollerShutterHw::setDirectionToClose()
{
   if ( independent )
   {
      direction = TO_CLOSE;
   }
   else
   {
      directionOutput.set();
   }
}

void RollerShutterHw::setDirectionToOpen()
{
   if ( independent )
   {
      direction = TO_OPEN;
   }
   else
   {
      directionOutput.clear();
   }
}
