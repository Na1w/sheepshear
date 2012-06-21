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
	int run_mask = 0, dict_idx = 0xfee;
	for (;;) {
		if (run_mask < 0x100) {
			// Start new run
			if (--size < 0)
				break;
			run_mask = *src++ | 0xff00;
		}
		bool bit = run_mask & 1;
		run_mask >>= 1;
		if (bit) {
			// Verbatim copy
			if (--size < 0)
				break;
			int c = *src++;
			dict[dict_idx++] = c;
			*dest++ = c;
			dict_idx &= 0xfff;
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
				dict[dict_idx++] = c;
				*dest++ = c;
				idx &= 0xfff;
				dict_idx &= 0xfff;
			}
		}
	}
}


// Decode parcels of ROM image (MacOS 9.X and even earlier)
void
DecodeParcels(const uint8 *src, uint8 *dest, int size)
{
	uint32 parcel_offset = 0x14;
	D(bug("Offset   Type Name\n"));
	while (parcel_offset != 0) {
		const uint32 *parcel_data = (uint32 *)(src + parcel_offset);
		uint32 next_offset = ntohl(parcel_data[0]);
		uint32 parcel_type = ntohl(parcel_data[1]);
		D(bug("%08x %c%c%c%c %s\n", parcel_offset,
			(parcel_type >> 24) & 0xff, (parcel_type >> 16) & 0xff,
			(parcel_type >> 8) & 0xff, parcel_type & 0xff, &parcel_data[6]));
		if (parcel_type == FOURCC('r', 'o', 'm', ' ')) {
		uint32 lzss_offset  = ntohl(parcel_data[2]);
		uint32 lzss_size = ((uintptr)src + next_offset)
			- ((uintptr)parcel_data + lzss_offset);
			DecodeLZSS((uint8 *)parcel_data + lzss_offset, dest, lzss_size);
		}
		parcel_offset = next_offset;
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
		uint32 image_offset, image_size;
		bool decode_info_ok = false;

		char *s = strstr((char *)data, "constant lzss-offset");
		if (s != NULL) {
			// Probably a plain LZSS compressed ROM image
			if (sscanf(s - 7, "%06x", &image_offset) == 1) {
				s = strstr((char *)data, "constant lzss-size");
				if (s != NULL && (sscanf(s - 7, "%06x", &image_size) == 1))
					decode_info_ok = true;
			}
		} else {
			// Probably a MacOS 9.2.x ROM image
			s = strstr((char *)data, "constant parcels-offset");
			if (s != NULL) {
				if (sscanf(s - 7, "%06x", &image_offset) == 1) {
					s = strstr((char *)data, "constant parcels-size");
					if (s != NULL && (sscanf(s - 7, "%06x", &image_size) == 1))
						decode_info_ok = true;
				}
			}
		}
		// No valid information to decode the ROM found?
		if (!decode_info_ok)
			return false;

		// Check signature, this could be a parcels-based ROM image
		uint32 rom_signature = ntohl(*(uint32 *)(data + image_offset));
		if (rom_signature == FOURCC('p', 'r', 'c', 'l')) {
			D(bug("Offset of parcels data: %08x\n", image_offset));
			D(bug("Size of parcels data: %08x\n", image_size));
			DecodeParcels(data + image_offset, result, image_size);
		} else {
			D(bug("Offset of compressed data: %08x\n", image_offset));
			D(bug("Size of compressed data: %08x\n", image_size));
			DecodeLZSS(data + image_offset, result, image_size);
		}
		return true;
	}
	return false;
}


bool
DecodeROMInfo(const char* fileName, romInfo *info)
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
		bug("%s: Invalid Mac ROM!\n");
		free(decodedROM);
		delete[] romBuffer;
		return false;
	}
	delete[] romBuffer;

	// Load results into romInfo
	info->checksum = ntohl(*(uint32 *)decodedROM);
	info->version = ntohs(*(uint16 *)(decodedROM + 8));
	info->subVersion = ntohs(*(uint16 *)(decodedROM + 18));
	info->nanokernelID = (char *)decodedROM + 0x30d064;
	info->resourceMapLocation = ntohl(*(uint32 *)(decodedROM + 26));
	info->trapTableLocation = ntohl(*(uint32 *)(decodedROM + 34));

	free(decodedROM);
	return true;
}
