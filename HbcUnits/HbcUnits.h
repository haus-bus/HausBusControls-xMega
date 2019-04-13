/*
 * HbcUnits.h
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#ifndef HwUnits_HwUnits_H
#define HwUnits_HwUnits_H

#include <DefaultTypes.h>
#include <Security/Checksum.h>
#include <DebugOptions.h>
#include <Protocols/HBCP.h>
#include <Tracing/Logger.h>
#include <Time/SystemTime.h>
#include <IResponse.h>
#include <EventPkg/EventPkg.h>
#include <CriticalSection.h>
#include <Reactive.h>
class BaseSensorUnit;

class Button;

class DS1820;

class DaliLine;

class Dht;

class DigitalPort;

class Dimmer;

class HwConfiguration;

class IDimmerHw;

class IrReceiver;

class Led;

class LogicalButton;

class PortPinUnit;

class RollerShutter;

class TwiStream;

class UartStream;

class UdpStream;

class Gateway;

class HbcIpStackManager;

class IResponse;

class ModBusSlave;

class PersistentRules;

class Rule;

class RuleElement;

class RuleEngine;

extern void* allocOnce( size_t size );

#endif
