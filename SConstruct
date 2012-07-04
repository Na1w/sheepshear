#####################################################
#
# SheepShear, A PowerPC Mac emulator
# Forked from SheepShaver
#
# scons build script, all roads lead to Rome
# 2012, Alexander von Gluck
#
#####################################################
import SCons

build_dir = 'build-release'
SConscript('src/SConscript', variant_dir=build_dir, duplicate=0)
Clean('.', build_dir)
