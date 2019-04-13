/*
 * SystemBoards.h
 *
 *  Created on: 17.07.2017
 *      Author: Viktor Pankraz
 */

#ifndef Electronics_SystemBoards_SystemBoards_H
#define Electronics_SystemBoards_SystemBoards_H

#include <DefaultTypes.h>

class HbcDeviceHw;

enum TRACE_PORT_PINS {
   TR_IDLE_PIN = Pin0,
   TR_INT_PIN = Pin1
};

#define MOD_ID_SECTION __attribute__( ( section( ".vectors" ) ) )

static const uint8_t MAX_SLOTS = 8;

#endif
