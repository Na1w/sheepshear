/*
 *  machine_defs.h - ROM and machine info
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
#ifndef __MACHINE_DEFS_H
#define __MACHINE_DEFS_H


#define CPU_UNKNOWN		(0 << 0)
#define CPU_68000		(1 << 0)
#define CPU_68010		(1 << 1)
#define CPU_68020		(1 << 2)
#define CPU_68030		(1 << 3)
#define CPU_68040		(1 << 4)
#define CPU_PPC601		(1 << 5)
#define CPU_PPC603		(1 << 6)
#define CPU_PPC604		(1 << 7)
#define CPU_PPC740		(1 << 8)
#define CPU_PPC745		(1 << 9)
#define CPU_PPC750		(1 << 10)
#define CPU_PPC755		(1 << 11)


struct knownMachines {
	uint32		romChecksum;
	uint32		romSize;
	bool		romSupported;
	uint32		cpuSupport;
	const char*	machineName;
} kKnownMachines[] = {
	{0x28BA61CE, 0x10000, 0, CPU_68000, "Mac 128"},
	{0x28BA4E50, 0x10000, 0, CPU_68000, "Mac 512"},
	{0x4D1F8172, 0x20000, 0, CPU_68000, "Mac Plus"},
	{0xB2E362A8, 0x40000, 0, CPU_68000, "Mac SE"},
	{0xB306E171, 0x40000, 0, CPU_68000, "Mac SE HDFD"},
	{0xA49F9914, 0x80000, 0, CPU_68000, "Mac Classic"},
	{0x3193670E, 0x80000, 0, CPU_68020 | CPU_68040, "Mac Classic II"},
	{0xECD99DC0, 0x100000, 0, CPU_68020 | CPU_68040, "Mac Color Classic"},
	{0x9779D2C4, 0x80000, 0, CPU_68020, "Mac II"},
	{0x97851DB6, 0x80000, 0, CPU_68020, "Mac II"},
	{0x97221136, 0x80000, 0, CPU_68020 | CPU_68030, "Mac IIx"},
	{0x49579803, 0x100000, 0, CPU_68020 | CPU_68030, "Mac IIvx"},
	{0x3193670E, 0x100000, 0, CPU_68020 | CPU_68030, "Mac IIvi"},
	{0x35C28C8F, 0x80000, 0, CPU_68020 | CPU_68030, "Mac IIxi"},
	{0x368CADFE, 0x80000, 0, CPU_68020 | CPU_68040, "Mac IIci"},
	{0x36B7FB6C, 0x80000, 0, CPU_68020 | CPU_68040, "Mac IIsi"},
	{0x4147DD77, 0x80000, 0, CPU_68020 | CPU_68040, "Mac IIfx"},
	{0x350EACF0, 0x80000, 0, CPU_68020 | CPU_68040, "Mac LC"},
	{0x35C28F5F, 0x80000, 0, CPU_68020 | CPU_68040, "Mac LC II"},
	{0xECBBC41C, 0x100000, 0, CPU_68020 | CPU_68040, "Performa 460"},
	{0xFF7439EE, 0x100000, 0, CPU_68020 | CPU_68040, "Performa 475"},
	{0x064DC91D, 0x100000, 0, CPU_68020 | CPU_68040, "Performa 580"},
	{0x06684214, 0x100000, 0, CPU_68020 | CPU_68040, "Performa 630"},
	{0xF1A6F343, 0x100000, 0, CPU_68020 | CPU_68040, "Quadra 610"},
	{0xF1ACAD13, 0x100000, 0, CPU_68020 | CPU_68040, "Quadra 650"},
	{0x5BF10FD1, 0x200000, 0, CPU_68040, "Quadra 660"},
	{0x420DBFF3, 0x100000, 0, CPU_68020 | CPU_68040, "Quadra 900"},
	{0x3DC27823, 0x100000, 0, CPU_68020 | CPU_68040, "Quadra 950"},
	{0x96CD923D, 0x100000, 0, CPU_68020 | CPU_68040, "Quadra ???"},
	{0xE33B2724, 0x100000, 0, CPU_68020 | CPU_68040, "PowerBook 165"},
	{0x63ABFD3F, 0x400000, 0, CPU_PPC603, "PowerBook 5300"},
	{0x4A6F7921, 0x400000, 0, CPU_PPC603, "Power Mac 5400"},
	{0x2BF65931, 0x400000, 0, CPU_PPC603, "Bandai Pippin (Kinka Dev)"},
	{0x2BEF21B7, 0x400000, 0, CPU_PPC603, "Bandai Pippin (Kinka 1.0)"},
	{0x9FEB69B3, 0x400000, 0, CPU_PPC601, "Power Mac 6100"},
	{0x4E208E15, 0x400000, 0, CPU_PPC601, "Power Mac 7200"},
	{0x9630C68B, 0x400000, 0, CPU_PPC601, "Power Mac 7200"},
	{0x960E4BE9, 0x400000, 0, CPU_PPC604, "Power Mac 8600"},
	{0x6F5724C0, 0x400000, 0, CPU_PPC604, "Performa 6400"},
	{0x95CD923D, 0x400000, 0, CPU_UNKNOWN, "Power Mac Unknown"},
	{0xCBB01212, 0x400000, 0, CPU_PPC740, "PowerBook G3 Wallstreet"},
	{0x79D68D63, 0x400000, 0, CPU_PPC750, "Power Mac G3"},
	{0x78F57389, 0x400000, 0, CPU_PPC750, "Power Mac G3"}
};


static int
ModelChecksumLookup(uint32 checksum)
{
	// check for known ROM checksum
	int i;
	for (i = 0; i < sizeof(kKnownMachines)
		/ sizeof(kKnownMachines[0]); i++) {
		if (kKnownMachines[i].romChecksum == checksum)
			return i;
	}
	return -1;
}


#endif /* __MACHINE_DEFS_H */
