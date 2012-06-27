/*
 *  rom_patches.h - ROM patches
 *
 *  SheepShaver (C) 1997-2008 Christian Bauer and Marc Hellwig
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
#ifndef ROM_PATCHES_H
#define ROM_PATCHES_H


// ROM types
enum {
	ROMTYPE_TNT,		// Power Macintosh 7200 '94
	ROMTYPE_PIP,		// Bandai Pippin '95
	ROMTYPE_CORDYCEPS,	// Power Macintosh 6300 '96
	ROMTYPE_ALCHEMY,	// Power Macintosh 6400 '96
	ROMTYPE_ZANZIBAR,	// ???
	ROMTYPE_PBX,		// PowerBook 1400cs '96
	ROMTYPE_GAZELLE,	// Power Macintosh 6500 '97
	ROMTYPE_GOSSAMER,	// Power Macintosh G3 '97
	ROMTYPE_GRX,		// PowerBook G3 Wallstreet '97
	ROMTYPE_NEWWORLD
};
extern int ROMType;

extern bool PatchROM(void);
extern void InstallDrivers(void);

extern void AddSifter(uint32 type, int16 id);
extern bool FindSifter(uint32 type, int16 id);


#endif
