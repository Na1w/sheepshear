/*
 *  xpram_unix.cpp - XPRAM handling, Unix specific stuff
 *
 *  Basilisk II (C) 1997-2008 Christian Bauer
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

#include <stdlib.h>
#include "xpram.h"

#define DEBUG 0
#include "debug.h"


// XPRAM file name and path
#if POWERPC_ROM
const char XPRAM_FILE_NAME[] = ".sheepshear_nvram";
#else
const char XPRAM_FILE_NAME[] = ".sheepshear_xpram";
#endif


/*
 *  Load XPRAM from settings file
 */
void
MacPRAM::Load(const char* basedir)
{
	if (basedir != NULL) {
#if POWERPC_ROM
		snprintf(fPRAMFile, PATH_MAX, "%s/nvram", basedir);
#else
		snprintf(fPRAMFile, PATH_MAX, "%s/xpram", basedir);
#endif
	} else {
		// Construct XPRAM path
		fPRAMFile[0] = 0;
		char *home = getenv("HOME");
		if (home != NULL && strlen(home) < 1000) {
			strncpy(fPRAMFile, home, 1000);
			strcat(fPRAMFile, "/");
		}
		strcat(fPRAMFile, XPRAM_FILE_NAME);
	}

	D(bug("%s: %s\n", __func__, fPRAMFile));

	// Load XPRAM from settings file
	int fd;
	if ((fd = open(fPRAMFile, O_RDONLY)) >= 0) {
		read(fd, fPRAM, XPRAM_SIZE);
		close(fd);
	} else
		bug("%s: failed to open %s\n", __func__, fPRAMFile);
}


/*
 *  Save XPRAM to settings file
 */
void
MacPRAM::Save()
{
	int fd;
	if ((fd = open(fPRAMFile, O_WRONLY | O_CREAT, 0666)) >= 0) {
		D(bug("%s: %s\n", __func__, fPRAMFile));
		write(fd, fPRAM, XPRAM_SIZE);
		close(fd);
	} else
		bug("%s: failed to create %s\n", __func__, fPRAMFile);
}


/*
 *  Delete PRAM file
 */
void
MacPRAM::Zap()
{
	if (fPRAMFile != NULL) {
		D(bug("%s: erasing '%s'\n", __func__, fPRAMFile));
		// Delete file
		unlink(fPRAMFile);
	}
}
