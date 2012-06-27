/*
 *  rom_patches.cpp - ROM patches
 *
 *  SheepShear, 2012 Alexander von Gluck
 *  Portions from SheepShaver (C) 1997-2008 Christian Bauer and Marc Hellwig
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef ROM_TOOLBOX_H
#define ROM_TOOLBOX_H


#include "sysdeps.h"


struct romInfo {
	uint32	checksum;
	uint16	version;
	uint16	subVersion;
	char*	nanokernelID;
	uint32	resourceMapLocation;
	uint32	trapTableLocation;
};


void DecodeLZSS(const uint8 *src, uint8 *dest, int size);
void DecodeParcels(const uint8 *src, uint8 *dest, int size);
bool DecodeROM(uint8 *data, uint32 size, uint8 *result);
bool DecodeROMInfo(const char* fileName, romInfo *info);


#endif /* ROM_TOOLBOX_H */