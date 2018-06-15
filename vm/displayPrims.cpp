/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// Copyright 2018 John Maloney, Bernat Romagosa, and Jens Mönig

// displayPrims.c - 5x5 LED display and NeoPixel primitives
// John Maloney, May 2018

#include <Arduino.h>
#include <stdio.h>

#include "mem.h"
#include "interp.h"

// LED Matrix Pins on BBC micro:bit and Calliope

#if defined(ARDUINO_BBC_MICROBIT)

#define ROW1 3
#define ROW2 4
#define ROW3 6
#define ROW4 7
#define ROW5 9
#define ROW6 10
#define ROW7 23
#define ROW8 24
#define ROW9 25

#define COL1 26
#define COL2 27
#define COL3 28

#elif defined(ARDUINO_CALLIOPE)

#define ROW1 4
#define ROW2 5
#define ROW3 12
#define ROW4 11
#define ROW5 10
#define ROW6 6
#define ROW7 7
#define ROW8 8
#define ROW9 9

#define COL1 13
#define COL2 14
#define COL3 15

#endif

static int microBitDisplayBits = 0;

#if defined(ARDUINO_BBC_MICROBIT) || defined(ARDUINO_CALLIOPE)

static void turnDisplayOn() {
	char pins[] = {COL1, COL2, COL3, ROW1, ROW2, ROW3, ROW4, ROW5, ROW6, ROW7, ROW8, ROW9};

	for (int i = 0; i < 12; i++) setPinMode(pins[i], OUTPUT);
}

static void turnDisplayOff() {
	char pins[] = {COL1, COL2, COL3, ROW1, ROW2, ROW3, ROW4, ROW5, ROW6, ROW7, ROW8, ROW9};

	for (int i = 0; i < 12; i++) setPinMode(pins[i], INPUT);
}

static int displaySnapshot = 0;
static int displayCycle = 0;

#define DISPLAY_BIT(n) (((displaySnapshot >> (n - 1)) & 1) ? LOW : HIGH)

void updateMicrobitDisplay() {
	// Update the display by cycling through the three columns, turning on the rows
	// for each column. To minimize display artifacts, the display bits are snapshot
	// at the start of each cycle and the snapshot is not changed during the cycle.

	if (!microBitDisplayBits && !displaySnapshot) return; // display is off

	if (0 == displayCycle) { // starting a new cycle
		if (!displaySnapshot && microBitDisplayBits) turnDisplayOn(); // display just became on
		if (displaySnapshot && !microBitDisplayBits) { // display just became off
			displaySnapshot = 0;
			turnDisplayOff();
			return;
		}
		// take a snapshot of the display bits for the next cycle
		displaySnapshot = microBitDisplayBits;
	}

	// turn off all columns
	digitalWrite(COL1, LOW);
	digitalWrite(COL2, LOW);
	digitalWrite(COL3, LOW);

	switch (displayCycle) {
	case 0:
		digitalWrite(ROW1, DISPLAY_BIT(1));
		digitalWrite(ROW2, DISPLAY_BIT(3));
		digitalWrite(ROW6, DISPLAY_BIT(5));
		digitalWrite(ROW3, DISPLAY_BIT(12));
		digitalWrite(ROW4, DISPLAY_BIT(16));
		digitalWrite(ROW5, DISPLAY_BIT(17));
		digitalWrite(ROW9, DISPLAY_BIT(18));
		digitalWrite(ROW8, DISPLAY_BIT(19));
		digitalWrite(ROW7, DISPLAY_BIT(20));
		digitalWrite(COL1, HIGH);
		break;
	case 1:
		digitalWrite(ROW7, DISPLAY_BIT(2));
		digitalWrite(ROW8, DISPLAY_BIT(4));
		digitalWrite(ROW2, DISPLAY_BIT(11));
		digitalWrite(ROW6, DISPLAY_BIT(13));
		digitalWrite(ROW1, DISPLAY_BIT(15));
		digitalWrite(ROW5, DISPLAY_BIT(22));
		digitalWrite(ROW9, DISPLAY_BIT(24));
		digitalWrite(ROW3, HIGH); // unused
		digitalWrite(ROW4, HIGH); // unused
		digitalWrite(COL2, HIGH);
		break;
	case 2:
		digitalWrite(ROW7, DISPLAY_BIT(6));
		digitalWrite(ROW8, DISPLAY_BIT(7));
		digitalWrite(ROW9, DISPLAY_BIT(8));
		digitalWrite(ROW5, DISPLAY_BIT(9));
		digitalWrite(ROW4, DISPLAY_BIT(10));
		digitalWrite(ROW3, DISPLAY_BIT(14));
		digitalWrite(ROW6, DISPLAY_BIT(21));
		digitalWrite(ROW1, DISPLAY_BIT(23));
		digitalWrite(ROW2, DISPLAY_BIT(25));
		digitalWrite(COL3, HIGH);
		break;
	}
	displayCycle = (displayCycle + 1) % 3;
}

#else

void updateMicrobitDisplay() { } // stub for boards without 5x5 LED displays

#endif

// Display Primitives for micro:bit/Calliope (noops on other boards)

void primMBDisplay(OBJ *args) {
	OBJ arg = args[0];
	if (isInt(arg)) microBitDisplayBits = evalInt(arg);
}

void primMBDisplayOff(OBJ *args) {
	OBJ off = falseObj;
	primSetUserLED(&off);
	microBitDisplayBits = 0;
}

void primMBPlot(OBJ *args) {
	int x = evalInt(args[0]);
	int y = evalInt(args[1]);
	if ((1 <= x) && (x <= 5) && (1 <= y) && (y <= 5)) {
		int shift = (5 * (y - 1)) + (x - 1);
		microBitDisplayBits |= (1 << shift);
	}
}

void primMBUnplot(OBJ *args) {
	int x = evalInt(args[0]);
	int y = evalInt(args[1]);
	if ((1 <= x) && (x <= 5) && (1 <= y) && (y <= 5)) {
		int shift = (5 * (y - 1)) + (x - 1);
		microBitDisplayBits &= ~(1 << shift);
	}
}

// NeoPixel Support

#define DELAY_CYCLES(n) { \
	__asm__ __volatile__ ( \
		".rept " #n " \n\t" \
		"nop \n\t" \
		".endr \n\t" \
	); \
}

inline uint32 saveIRQState(void) {
	uint32 pmask = 0;
	#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP32)
		__asm__ volatile ("rsil %0, #2" : "=a" (pmask));
	#else
		pmask = __get_PRIMASK() & 1;
		__set_PRIMASK(1);
	#endif
	return pmask;
}

inline void restoreIRQState(uint32 pmask) {
	#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP32)
		 __asm__ volatile ("wsr %0, ps; rsync" :: "a" (pmask));
	#else
		__set_PRIMASK(pmask);
	#endif
}

int neoPixelBits = 24;
int neoPixelPinMask = 0;
volatile int *neoPixelPinSet = NULL;
volatile int *neoPixelPinClr = NULL;

#if defined(ARDUINO_BBC_MICROBIT) || defined(ARDUINO_CALLIOPE)

#define GPIO_SET 0x50000508
#define GPIO_CLR 0x5000050C
#define GPIO_SET_DIR 0x50000518

static void initNeoPixelPin(int pinNum) {
	if ((pinNum < 0) || (pinNum >= PINS_COUNT)) {
		#if defined(ARDUINO_BBC_MICROBIT)
			pinNum = 0; // use left exteranl pad on micro:bit
		#else
			pinNum = 26; // internal NeoPixel pin on Calliope
		#endif
	}
	neoPixelPinMask = 1 << g_ADigitalPinMap[pinNum];
	neoPixelPinSet = (int *) GPIO_SET;
	neoPixelPinClr = (int *) GPIO_CLR;

	volatile int *neoPixelPinSetDir = (int *) GPIO_SET_DIR;
	*neoPixelPinSetDir = neoPixelPinMask;
}

static void sendNeoPixelData(int val) { // micro:bit/Calliope (16 MHz)
	// Note: This code is timing sensitive and the timing changes in unpredictable
	// ways with code changes. For example, the zero-bit code uses constant register
	// addresses while the one-bit uses the indirect variables. Making those
	// consistent (either way) increases the timing. Go figure! Thus, if you change
	// this code in any way, be sure to check the timing with an oscilloscope.
	uint32 oldIRQ = saveIRQState();
	for (uint32 mask = (1 << (neoPixelBits - 1)); mask > 0; mask >>= 1) {
		if (val & mask) { // one bit; timing goal: high 900 nsecs, low 350 nsecs
			*neoPixelPinSet = neoPixelPinMask;
			DELAY_CYCLES(8);
			*neoPixelPinClr = neoPixelPinMask;
		} else { // zero bit; timing goal: high 350 nsecs, low 800 nsecs
			// This addressing mode gave the shortest pulse width.
			*((int *) GPIO_SET) = neoPixelPinMask;
			*((int *) GPIO_CLR) = neoPixelPinMask;
		}
	}
	restoreIRQState(oldIRQ);
}

#elif defined(ARDUINO_ARCH_SAMD)

#define PORT_BASE 0x41004400

static void initNeoPixelPin(int pinNum) {
	if ((pinNum < 0) || (pinNum >= PINS_COUNT)) {
		#ifdef ARDUINO_SAMD_CIRCUITPLAYGROUND_EXPRESS
			pinNum = 8; // internal NeoPixel pin
		#else
			pinNum = 0; // default to pin 0
		#endif
	}

	neoPixelPinMask = 1 << g_APinDescription[pinNum].ulPin;
	int baseReg = PORT_BASE + (0x80 * g_APinDescription[pinNum].ulPort);
	neoPixelPinSet = (int *) (baseReg + 0x18);
	neoPixelPinClr = (int *) (baseReg + 0x14);

	volatile int *neoPixelPinSetDir = (int *) (baseReg + 0x08);
	*neoPixelPinSetDir = neoPixelPinMask;
}

static void sendNeoPixelData(int val) { // SAMD21 (48 MHz)
	uint32 oldIRQ = saveIRQState();
	for (uint32 mask = (1 << (neoPixelBits - 1)); mask > 0; mask >>= 1) {
		if (val & mask) { // one bit; timing goal: high 900 nsecs, low 350 nsecs
			*neoPixelPinSet = neoPixelPinMask;
			DELAY_CYCLES(19);
			*neoPixelPinClr = neoPixelPinMask;
			DELAY_CYCLES(0);
		} else { // zero bit; timing goal: high 350 nsecs, low 800 nsecs
			*neoPixelPinSet = neoPixelPinMask;
			DELAY_CYCLES(0);
			*neoPixelPinClr = neoPixelPinMask;
			DELAY_CYCLES(20);
		}
	}
	restoreIRQState(oldIRQ);
}

#else // stub for boards without NeoPixels

static void initNeoPixelPin(int pinNum) { }
static void sendNeoPixelData(int val) { }

#endif // NeoPixel Support

const int whiteTable[64] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18, 20, 22, 24, 26, 28, 30, 32,
	35, 38, 41, 44, 47, 50, 54, 58, 62, 66, 70, 75, 80, 85, 90, 97, 104, 111, 118, 125, 132,
	139, 146, 153, 160, 167, 174, 181, 188, 195, 202, 209, 216, 223, 230, 237, 244, 251, 255};

void primNeoPixelSend(OBJ *args) {
	if (!neoPixelPinSet) initNeoPixelPin(-1); // if pin not set, use the internal NeoPixel pin

	OBJ arg = args[0];
	if (isInt(arg)) {
		int rgb = obj2int(arg);
		// re-order RGB -> GBR (NeoPixel order)
		int val = ((rgb & 0xFF00) << 8) | ((rgb & 0xFF0000) >> 8) | (rgb & 0xFF);
		if (32 == neoPixelBits) { // send white as the final byte of four
			val = (val << 8) | whiteTable[(rgb >> 24) & 0x3F];
		}
		sendNeoPixelData(val);
	} else if (IS_CLASS(arg, ArrayClass)) {
		int count = objWords(arg);
		for (int i = 0; i < count; i++) {
			OBJ item = FIELD(arg, i);
			if (isInt(item)) {
				int rgb = obj2int(item);
				// re-order RGB -> GBR (NeoPixel order)
				int val = ((rgb & 0xFF00) << 8) | ((rgb & 0xFF0000) >> 8) | (rgb & 0xFF);
				if (32 == neoPixelBits) { // send white as the final byte of four
					val = (val << 8) | whiteTable[(rgb >> 24) & 0x3F];
				}
				sendNeoPixelData(val);
			}
		}
	}
}

void primNeoPixelSetPin(int argCount, OBJ *args) {
	int pinNum = isInt(args[0]) ? obj2int(args[0]) : -1; // -1 means "internal NeoPixel pin"
	neoPixelBits = ((argCount > 1) && (trueObj == args[1])) ? 32 : 24;
	initNeoPixelPin(pinNum);
}

// MicroBit Font

// From the Lancaster University MicroBit library (under the MIT license):
// https://github.com/lancaster-university/microbit-dal/blob/master/source/core/MicroBitFont.cpp
//
// Each 5x5 character is represented by the lower five bits of five bytes (top to bottom rows
// of the LED matrix). There are 95 characters. The first  is the space character (ASCII 32)
// the last is the tilde (ASCII 126, '~').

const unsigned char pendolino3[475] = {
0x0, 0x0, 0x0, 0x0, 0x0, 0x8, 0x8, 0x8, 0x0, 0x8, 0xa, 0x4a, 0x40, 0x0, 0x0, 0xa, 0x5f, 0xea, 0x5f, 0xea, 0xe, 0xd9, 0x2e, 0xd3, 0x6e, 0x19, 0x32, 0x44, 0x89, 0x33, 0xc, 0x92, 0x4c, 0x92, 0x4d, 0x8, 0x8, 0x0, 0x0, 0x0, 0x4, 0x88, 0x8, 0x8, 0x4, 0x8, 0x4, 0x84, 0x84, 0x88, 0x0, 0xa, 0x44, 0x8a, 0x40, 0x0, 0x4, 0x8e, 0xc4, 0x80, 0x0, 0x0, 0x0, 0x4, 0x88, 0x0, 0x0, 0xe, 0xc0, 0x0, 0x0, 0x0, 0x0, 0x8, 0x0, 0x1, 0x22, 0x44, 0x88, 0x10, 0xc, 0x92, 0x52, 0x52, 0x4c, 0x4, 0x8c, 0x84, 0x84, 0x8e, 0x1c, 0x82, 0x4c, 0x90, 0x1e, 0x1e, 0xc2, 0x44, 0x92, 0x4c, 0x6, 0xca, 0x52, 0x5f, 0xe2, 0x1f, 0xf0, 0x1e, 0xc1, 0x3e, 0x2, 0x44, 0x8e, 0xd1, 0x2e, 0x1f, 0xe2, 0x44, 0x88, 0x10, 0xe, 0xd1, 0x2e, 0xd1, 0x2e, 0xe, 0xd1, 0x2e, 0xc4, 0x88, 0x0, 0x8, 0x0, 0x8, 0x0, 0x0, 0x4, 0x80, 0x4, 0x88, 0x2, 0x44, 0x88, 0x4, 0x82, 0x0, 0xe, 0xc0, 0xe, 0xc0, 0x8, 0x4, 0x82, 0x44, 0x88, 0xe, 0xd1, 0x26, 0xc0, 0x4, 0xe, 0xd1, 0x35, 0xb3, 0x6c, 0xc, 0x92, 0x5e, 0xd2, 0x52, 0x1c, 0x92, 0x5c, 0x92, 0x5c, 0xe, 0xd0, 0x10, 0x10, 0xe, 0x1c, 0x92, 0x52, 0x52, 0x5c, 0x1e, 0xd0, 0x1c, 0x90, 0x1e, 0x1e, 0xd0, 0x1c, 0x90, 0x10, 0xe, 0xd0, 0x13, 0x71, 0x2e, 0x12, 0x52, 0x5e, 0xd2, 0x52, 0x1c, 0x88, 0x8, 0x8, 0x1c, 0x1f, 0xe2, 0x42, 0x52, 0x4c, 0x12, 0x54, 0x98, 0x14, 0x92, 0x10, 0x10, 0x10, 0x10, 0x1e, 0x11, 0x3b, 0x75, 0xb1, 0x31, 0x11, 0x39, 0x35, 0xb3, 0x71, 0xc, 0x92, 0x52, 0x52, 0x4c, 0x1c, 0x92, 0x5c, 0x90, 0x10, 0xc, 0x92, 0x52, 0x4c, 0x86, 0x1c, 0x92, 0x5c, 0x92, 0x51, 0xe, 0xd0, 0xc, 0x82, 0x5c, 0x1f, 0xe4, 0x84, 0x84, 0x84, 0x12, 0x52, 0x52, 0x52, 0x4c, 0x11, 0x31, 0x31, 0x2a, 0x44, 0x11, 0x31, 0x35, 0xbb, 0x71, 0x12, 0x52, 0x4c, 0x92, 0x52, 0x11, 0x2a, 0x44, 0x84, 0x84, 0x1e, 0xc4, 0x88, 0x10, 0x1e, 0xe, 0xc8, 0x8, 0x8, 0xe, 0x10, 0x8, 0x4, 0x82, 0x41, 0xe, 0xc2, 0x42, 0x42, 0x4e, 0x4, 0x8a, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1f, 0x8, 0x4, 0x80, 0x0, 0x0, 0x0, 0xe, 0xd2, 0x52, 0x4f, 0x10, 0x10, 0x1c, 0x92, 0x5c, 0x0, 0xe, 0xd0, 0x10, 0xe, 0x2, 0x42, 0x4e, 0xd2, 0x4e, 0xc, 0x92, 0x5c, 0x90, 0xe, 0x6, 0xc8, 0x1c, 0x88, 0x8, 0xe, 0xd2, 0x4e, 0xc2, 0x4c, 0x10, 0x10, 0x1c, 0x92, 0x52, 0x8, 0x0, 0x8, 0x8, 0x8, 0x2, 0x40, 0x2, 0x42, 0x4c, 0x10, 0x14, 0x98, 0x14, 0x92, 0x8, 0x8, 0x8, 0x8, 0x6, 0x0, 0x1b, 0x75, 0xb1, 0x31, 0x0, 0x1c, 0x92, 0x52, 0x52, 0x0, 0xc, 0x92, 0x52, 0x4c, 0x0, 0x1c, 0x92, 0x5c, 0x90, 0x0, 0xe, 0xd2, 0x4e, 0xc2, 0x0, 0xe, 0xd0, 0x10, 0x10, 0x0, 0x6, 0xc8, 0x4, 0x98, 0x8, 0x8, 0xe, 0xc8, 0x7, 0x0, 0x12, 0x52, 0x52, 0x4f, 0x0, 0x11, 0x31, 0x2a, 0x44, 0x0, 0x11, 0x31, 0x35, 0xbb, 0x0, 0x12, 0x4c, 0x8c, 0x92, 0x0, 0x11, 0x2a, 0x44, 0x98, 0x0, 0x1e, 0xc4, 0x88, 0x1e, 0x6, 0xc4, 0x8c, 0x84, 0x86, 0x8, 0x8, 0x8, 0x8, 0x8, 0x18, 0x8, 0xc, 0x88, 0x18, 0x0, 0x0, 0xc, 0x83, 0x60};

OBJ primMBShapeForLetter(OBJ *args) {
	// Return a 25-bit integer representing the 5x5 shape for a letter from the micro:bit font.
	// Note: MicroBlocks numbers the bits of the LED display starting at the top
	// left and scanning from left-to-right. Thus, the top-left LED of the display
	// corresponds to the least significant bit (2 to the 0) of the shape word while
	// the bottom-right LED is the 25th bit (2 to the 24).

	int ascii = 0;
	OBJ arg = args[0];
	if (isInt(arg)) {
		// argument is an integer
		ascii = evalInt(arg);
	} else if (IS_CLASS(arg, StringClass) && (objWords(arg) > 0)) {
		// argument is a non-empty string; use its first (and usually only) byte
		uint8 *bytes = (uint8 *) &FIELD(arg, 0);
		ascii = bytes[0];
	}
	if ((ascii < 32) || (ascii > 126)) return int2obj(0); // out of range; return empty shape
	int result = 0, resultBit = 1;
	int firstRow = 5 * (ascii - 32);
	for (int row = firstRow; row < (firstRow + 5); row++) {
		int rowBits = pendolino3[row];
		for (int mask = 0x10; mask > 0; mask = (mask >> 1)) {
			// mask selects the bits of the font row byte from left-to-right and sets
			// the next result bit if the font row has one at that location
			// Note: AMicroBlocks glphys
			if (rowBits & mask) result += resultBit;
			resultBit = resultBit << 1;
		}
	}
	return int2obj(result);
}

void primMBDrawShape(int argCount, OBJ *args) {
	// Draw a 5x5 at the given x,y position, where 1,1 is the top-left of the LED display.
	// For example, x = 2 is shifted one column to the right and y = 3 is shifted two rows down.
	// Negative offsets are allowed. Only the part of the shape that overlaps the display will
	// be drawn. If argCount < 3, x and y default to 1.

	int shape = evalInt(args[0]);
	int x = 1, y = 1;
	if (argCount >= 3) {
		x = evalInt(args[1]);
		y = evalInt(args[2]);
	}
	int srcMask = 1;
	for (int dstY = y; dstY < (y + 5); dstY++) {
		for (int dstX = x; dstX < (x + 5); dstX++) {
			if ((1 <= dstY) && (dstY <= 5) && (1 <= dstX) && (dstX <= 5)) {
				int shift = (5 * (dstY - 1)) + (dstX - 1);
				if (shape & srcMask) {
					microBitDisplayBits |= (1 << shift); // plot
				} else {
					microBitDisplayBits &= ~(1 << shift); // unplot
				}
			}
			srcMask <<= 1; // advance to next bit of shape
		}
	}
}