How do I compile SheepShear?
============================
SheepShear is a compex emulator supporting a large number of platforms.
Instructions below detail the steps for each.

The instructions below may be subject to change given SheepShear's rapid pace of change.

Linux gcc
---------
 * cd src/Unix
 * ./autogen.sh
 * make

Linux clang
-----------
 * cd src/Unix
 * CC=clang CXX=clang++ ./autogen.sh --disable-jit

MacOS X
-------
 * cd src/Unix
 * ./autogen.sh --x-includes=/Developer/SDKs/MacOSX10.6.sdk/usr/X11/include --x-libraries=/Developer/SDKs/MacOSX10.6.sdk/usr/X11/lib --without-gtk

Haiku
-----
 * cd src/Haiku
 * make
