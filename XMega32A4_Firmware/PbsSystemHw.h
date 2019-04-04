/*
 * PbsSystemHw.h
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#ifndef PbsSystemHw_H
#define PbsSystemHw_H

#include <HwUnitBoards/SystemBoards.h>
#include <HbcDeviceHw.h>
#include <InternalEeprom.h>

extern MOD_ID_SECTION const ModuleId moduleId;

class PbsSystemHw : public HbcDeviceHw
{
   ////    Constructors and destructors    ////

   public:

      PbsSystemHw();

      ////    Operations    ////

      void configure();

      void configureLogicalButtons();

      void configureTwi();

      void configureRs485();

      ////    Additional operations    ////


      ////    Attributes    ////


      ////    Relations and components    ////

   protected:

};

#endif
