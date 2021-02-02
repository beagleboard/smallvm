/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// Copyright 2018 John Maloney, Bernat Romagosa, and Jens Mönig

// serialPrims.cpp - MicroBlocks serial connectivity primitives
// Bernat Romagosa, February 2021

// micro:bit v2 (and other nRF52 boards), all the Adafruit SAMD21 boards, the Clue, all the ESP32 boards, and new Raspberry Pi Pico

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <Arduino.h>

#include "mem.h"
#include "interp.h"

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_SAMD) || defined(NRF51) || defined(NRF52)

static OBJ primSerialOpen(int argCount, OBJ *args) {
	if (argCount < 1) return fail(notEnoughArguments);
	int baudRate = obj2int(args[0]);
	Serial2.begin(baudRate);
	return trueObj;
}

static OBJ primSerialClose(int argCount, OBJ *args) {
	Serial2.end();
	return trueObj;
}

static OBJ primSerialReadByte(int argCount, OBJ *args) {
	uint8 byteReceived;
	if (Serial2.available() > 0) {
		byteReceived = Serial2.read();
		return int2obj(byteReceived);
	} else {
		return int2obj(-1);
	}
}

static OBJ primSerialWriteByte(int argCount, OBJ *args) {
	if (argCount < 1) return fail(notEnoughArguments);
	uint8 byte = obj2int(args[0]);
	Serial2.write(byte);
	return trueObj;
}

#else // no HW support for a secondary serial connection

static OBJ primSerialOpen(int argCount, OBJ *args) { return falseObj; }
static OBJ primSerialClose(int argCount, OBJ *args) { return falseObj; }
static OBJ primSerialReadByte(int argCount, OBJ *args) { return falseObj; }
static OBJ primSerialWriteByte(int argCount, OBJ *args) { return falseObj; }

#endif

static PrimEntry entries[] = {
	{"serialOpen", primSerialOpen},
	{"serialClose", primSerialClose},
	{"serialReadByte", primSerialReadByte},
	{"serialWriteByte", primSerialWriteByte},
};

void addSerialPrims() {
	addPrimitiveSet("serial", sizeof(entries) / sizeof(PrimEntry), entries);
}
