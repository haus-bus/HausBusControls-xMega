/*
 * HwUnitBoards.h
 *
 *  Created on: 17.07.2017
 *      Author: Viktor Pankraz
 */

#ifndef HbcUnits_HwUnitBoards_H
#define HbcUnits_HwUnitBoards_H

#include <DefaultTypes.h>

class DaliDimmerHw;

class DigitalPortHw;

class RollerShutterHw;

class SlotHw;

enum FIRMWARE_IDS
{
   SIM = 0,
   AR8_ID,
   MS6_ID,
   SD6_ID,
   SD485_ID,
   MAX_FW_ID
};


enum BoardRevisions
{
   OLD_BOARDS = 20,
   FCKE_V4_0 = 40,


   // X192A3 Controller
   AR8_I2C = 30,
   AR8_RS485 = 40,

   // X32A4 Controller
   SD6 = 30,

   SD485_IO12 = 40,

   SD485_B1 = 41,
   SD485_B2 = 42,
   SD485_B4 = 44,
   SD485_B6 = 46,

   SD485_LC4_0V = 43,
   SD485_LC4_1V = 45,
};

enum SlotType
{
   UNUSED_SLOT,
   DIMMER,
   POWER_SOCKET,
   ROLLER_SHUTTER,
   DOUBLE_SWITCH = 0x07,
   DIMMER_30 = 0x0A,
   DIMMER_31 = 0x0B,
   MAX_SLOT_TYPES,

   NOT_CONFIGURED = 0xFF,
};

// Turn off a power load generates a current spike. So wait some time to prevent errors in communication.
static const uint16_t POWER_SWITCH_DELAY = 6;

#endif
