/*
 *  platform_video.h - Video platform defines
 *
 *  SheepShear, 2012 Alexander von Gluck IV
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
#ifndef _PLATFORM_VIDEO_H
#define _PLATFORM_VIDEO_H


#include "video_defs.h"

#include <MediaKit.h>


class PlatformVideo
{
public:
		// Required for PlatformVideo class
		bool				DeviceInit();
		void				DeviceShutdown();
		bool				DeviceOpen();
		void				DeviceClose();
		void				DeviceInterrupt();

		void				InstallAccel();

		void				DeviceQuitFullScreen();

protected:
		int16				ModeChange(VidLocals *csSave, uint32 ParamPtr);
};


#endif /* _PLATFORM_VIDEO */
