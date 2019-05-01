/*
 * main.cpp
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */
#include "PbsSystem.h"

#include <Peripherals/WatchDog.h>

int main( int argc, char* argv[] )
{
   int status = 0;
   {
      PbsSystem::start();
   }
   return status;
}

void notifyBusy()
{

}

void notifyIdle()
{

#ifndef _DEBUG_
   WatchDog::reset();
#endif

}
