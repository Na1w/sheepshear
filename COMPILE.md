How do I compile SheepShear?
============================
SheepShear is a compex emulator supporting a large number of platforms.

SheepShear uses the [SCons](http://scons.org) build system. SCons
enables SheepShear to build on a wide range of platforms with minimal
requirements.

Requirements
------------
-   A Compiler (gcc or clang)
-   Python
-   [SCons](http://scons.org)

Installing SCons
----------------
### Linux
-   Utilize your favorate package manager to install SCons

### MacOS X / Darwin
-   Download SCons from http://scons.org
-   Extract SCons and change to the SCons directory
-   `python setup.py install`

### Haiku
-   Install SCons from Haiku ports

### Windows
-   Install Python and SCons

Compiling
-------------------

### Basic Build
-   Enter the sheepshear directory
-   execute `scons` to build

### Using a different compiler
You can utilize a different compiler by defining CC and CXX
-   `CC=gcc CXX=g++ scons`
-   `CC=clang CXX=clang++ scons`

### Build options
-   Enable Debugging symbols `scons debug=1`
-   Enable SDL `scons sdl=1`
