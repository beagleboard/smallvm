/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// Copyright 2018 John Maloney, Bernat Romagosa, and Jens Mönig

// persistFile.cpp - Persistent file/non-volatile storage operations

#include <stdio.h>
#include <string.h>

#include "mem.h"
#include "persist.h"

#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP32)
// Persistent file operations for Espressif boards

#include "fileSys.h"

#define FILE_NAME "/ublockscode"

static File codeFile;

static void closeAndOpenCodeFile() {
	if (codeFile) codeFile.close();
	codeFile = myFS.open(FILE_NAME, "a");
}

extern "C" void initCodeFile(uint8 *flash, int flashByteCount) {
	#ifdef ESP8266
		myFS.begin();
	#else
		myFS.begin(true);
	#endif
	codeFile = myFS.open(FILE_NAME, "r");
	// read code file into simulated Flash:
	if (codeFile) codeFile.readBytes((char*) flash, flashByteCount);
	closeAndOpenCodeFile();
}

extern "C" void writeCodeFile(uint8 *code, int byteCount) {
	if (codeFile) codeFile.write(code, byteCount);
	closeAndOpenCodeFile();
}

extern "C" void writeCodeFileWord(int word) {
	if (codeFile) codeFile.write((uint8 *) &word, 4);
}

extern "C" void clearCodeFile(int cycleCount) {
	if (codeFile) codeFile.close();
	codeFile = myFS.open(FILE_NAME, "w"); // truncate file to zero length
	int headerWord = ('S' << 24) | cycleCount; // Header record, version 1
	writeCodeFileWord(headerWord);
	closeAndOpenCodeFile();
}

#endif
