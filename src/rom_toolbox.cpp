/*
 *  rom_toolbox.cpp - Mac ROM tools
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


#include "sysdeps.h"
#include "cpu_emulation.h"
#include "macos_util.h"
#include "rom_toolbox.h"

#define DEBUG 0
#include "debug.h"


// Decode LZSS data
void
DecodeLZSS(const uint8 *src, uint8 *dest, int size)
{
	char dict[0x1000];
	int runMask = 0;
	int dictIDX = 0xfee;
	for (;;) {
		if (runMask < 0x100) {
			// Start new run
			if (--size < 0)
				break;
			runMask = *src++ | 0xff00;
		}
		bool bit = runMask & 1;
		runMask >>= 1;
		if (bit) {
			// Verbatim copy
			if (--size < 0)
				break;
			int c = *src++;
			dict[dictIDX++] = c;
			*dest++ = c;
			dictIDX &= 0xfff;
		} else {
			// Copy from dictionary
			if (--size < 0)
				break;
			int idx = *src++;
			if (--size < 0)
				break;
			int cnt = *src++;
			idx |= (cnt << 4) & 0xf00;
			cnt = (cnt & 0x0f) + 3;
			while (cnt--) {
				char c = dict[idx++];
				dict[dictIDX++] = c;
				*dest++ = c;
				idx &= 0xfff;
				dictIDX &= 0xfff;
			}
		}
	}
}


// Decode parcels of ROM image (MacOS 9.X and even earlier)
void
DecodeParcels(const uint8 *src, uint8 *dest, int size)
{
	uint32 parcelOffset = 0x14;
	D(bug("Offset   Type Name\n"));
	while (parcelOffset != 0) {
		const uint32 *parcelData = (uint32 *)(src + parcelOffset);
		uint32 nextOffset = ntohl(parcelData[0]);
		uint32 parcelType = ntohl(parcelData[1]);
		D(bug("%08x %c%c%c%c %s\n", parcelOffset,
			(parcelType >> 24) & 0xff, (parcelType >> 16) & 0xff,
			(parcelType >> 8) & 0xff, parcelType & 0xff, &parcelData[6]));
		if (parcelType == FOURCC('r', 'o', 'm', ' ')) {
		uint32 lzssOffset  = ntohl(parcelData[2]);
		uint32 lzssSize = ((uintptr)src + nextOffset)
			- ((uintptr)parcelData + lzssOffset);
			DecodeLZSS((uint8 *)parcelData + lzssOffset, dest, lzssSize);
		}
		parcelOffset = nextOffset;
	}
}


/*
 *  Decode ROM image, 4 MB plain images or NewWorld images
 */
bool
DecodeROM(uint8 *data, uint32 size, uint8 *result)
{
	if (size == ROM_SIZE) {
		// Plain ROM image
		memcpy(result, data, ROM_SIZE);
		return true;
	} else if (strncmp((char *)data, "<CHRP-BOOT>", 11) == 0) {
		// CHRP compressed ROM image
		uint32 imageOffset;
		uint32 imageSize;
		bool decodeSuccess = false;

		char *s = strstr((char *)data, "constant lzss-offset");
		if (s != NULL) {
			// Probably a plain LZSS compressed ROM image
			if (sscanf(s - 7, "%06x", &imageOffset) == 1) {
				s = strstr((char *)data, "constant lzss-size");
				if (s != NULL && (sscanf(s - 7, "%06x", &imageSize) == 1))
					decodeSuccess = true;
			}
		} else {
			// Probably a MacOS 9.2.x ROM image
			s = strstr((char *)data, "constant parcels-offset");
			if (s != NULL) {
				if (sscanf(s - 7, "%06x", &imageOffset) == 1) {
					s = strstr((char *)data, "constant parcels-size");
					if (s != NULL && (sscanf(s - 7, "%06x", &imageSize) == 1))
						decodeSuccess = true;
				}
			}
		}
		// No valid information to decode the ROM found?
		if (!decodeSuccess)
			return false;

		// Check signature, this could be a parcels-based ROM image
		uint32 romSignature = ntohl(*(uint32 *)(data + imageOffset));
		if (romSignature == FOURCC('p', 'r', 'c', 'l')) {
			D(bug("Offset of parcels data: %08x\n", imageOffset));
			D(bug("Size of parcels data: %08x\n", imageSize));
			DecodeParcels(data + imageOffset, result, imageSize);
		} else {
			D(bug("Offset of compressed data: %08x\n", imageOffset));
			D(bug("Size of compressed data: %08x\n", imageSize));
			DecodeLZSS(data + imageOffset, result, imageSize);
		}
		return true;
	}
	return false;
}


/*
 * Decode ROM image into an easily parseable struct
 */
bool
GetROMInfo(const char* fileName, uint32 item, char* result)
{
	int romHandle = open(fileName, O_RDONLY);
	if (romHandle < 0) {
		bug("%s: Couldn't access %s!\n", __func__, fileName);
		return false;
	}

	uint32 romSize = lseek(romHandle, 0, SEEK_END);
	lseek(romHandle, 0, SEEK_SET);
	uint8 *romBuffer = new uint8[ROM_SIZE];
	uint32 actualSize = read(romHandle, (void *)romBuffer, ROM_SIZE);
	close(romHandle);

	// Decode Mac ROM
	uint8 *decodedROM = (uint8*)malloc(ROM_AREA_SIZE);
	if (!DecodeROM(romBuffer, actualSize, decodedROM)) {
		bug("%s: Invalid Mac ROM! (%s)\n", __func__, fileName);
		free(decodedROM);
		delete[] romBuffer;
		sprintf(result, "invalid");
		return false;
	}
	delete[] romBuffer;

	int length = 0;
	memset(result, ' ', ROM_INFO_FIELD_SIZE);
	switch (item) {
		case GET_ROM_CHECKSUM:
			length = 9;
			snprintf(result, ROM_INFO_FIELD_SIZE, "%08lX",
				ntohl(*(uint32 *)decodedROM));
			break;
		case GET_ROM_VERSION:
			length = 5;
			snprintf(result, ROM_INFO_FIELD_SIZE, "%04X",
				ntohs(*(uint16 *)(decodedROM + 8)));
			break;
		case GET_ROM_SUBVERSION:
			length = 5;
			snprintf(result, ROM_INFO_FIELD_SIZE, "%04X",
				ntohs(*(uint16 *)(decodedROM + 18)));
			break;
		case GET_ROM_NANOKERNEL:
			length = 17;
			snprintf(result, ROM_INFO_FIELD_SIZE, "%s",
				(char *)decodedROM + 0x30d064);
			for (int i = 0; i < ROM_INFO_FIELD_SIZE; i++) {
				// Lets give nanokernel id up to first non-ascii
				if (result[i] < 32 || result[i] > 126) {
					length = i - 1;
					break;
				}
			}
			break;
		case GET_ROM_RESOURCEMAP:
			length = 9;
			snprintf(result, ROM_INFO_FIELD_SIZE, "%08lX",
				ntohl(*(uint32 *)(decodedROM + 26)));
			break;
		case GET_ROM_TRAPTABLE:
			length = 9;
			snprintf(result, ROM_INFO_FIELD_SIZE, "%08lX",
				ntohl(*(uint32 *)(decodedROM + 34)));
			break;
	}

	// Put end of line at determined length point
	result[length + 1] = '\0';

	// Clean up any remaining non-ascii data
	for (int i = 0; i < length; i++) {
		if (result[i] < 32 || result[i] > 126) {
			// Erase non-ascii char
			result[i] = ' ';
		}
	}

	free(decodedROM);
	return true;
}
