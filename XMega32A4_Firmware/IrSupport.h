/*
 * IrSupport.h
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#ifndef Electronics_SystemBoards_MS6_MS6_Boards_IrSupport_IrSupport_H
#define Electronics_SystemBoards_MS6_MS6_Boards_IrSupport_IrSupport_H

#include <DefaultTypes.h>
#include <Protocols/IrDecoder.h>

class PortPin;

void configureInfraRedHw( PortPin portPin, IrDecoder* decoder );

#endif
