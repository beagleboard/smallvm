#!/bin/sh
# Build uBlocks for generic GNU/Linux
# Connect to it via pseudo terminal
#
# Prereqs for 64-bit Linux systems:
#	libc6-dev-i386 gcc-multilib g++-multilib libsdl2-dev:i386 libsdl2-ttf-dev:i386

gcc -m32 -std=c99 -Wall -Wno-unused-variable -Wno-unused-result -O3 \
	-D GNUBLOCKS \
	-I/usr/local/include/SDL2 \
	-I ../vm \
	linux.c ../vm/*.c \
	linuxSensorPrims.c linuxFilePrims.c linuxIOPrims.c linuxNetPrims.c \
	linuxOutputPrims.c linuxTftPrims.c \
	-lSDL2 -lSDL2_ttf \
	-ldl -lm -lpthread -lsndio -lz \
	-o GnuBlocks

# For static linking replace -lSDL2 -lSDL2_ttf with:
#	/usr/local/lib/libSDL2.a /usr/local/lib/libSDL2_ttf.a /usr/local/lib/libfreetype.a \
