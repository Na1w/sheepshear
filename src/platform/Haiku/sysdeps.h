/*
 *  sysdeps.h - System dependent definitions for BeOS
 *
 *  SheepShear, 2012 Alexander von Gluck IV
 *  Rewritten from SheepShaver (C) 1997-2008 Christian Bauer and Marc Hellwig
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
#ifndef SYSDEPS_H
#define SYSDEPS_H


// Do we have std namespace?
#ifdef __powerpc__
#define NO_STD_NAMESPACE
#endif

#include <assert.h>
#include <interface/GraphicsDefs.h>
#include <KernelKit.h>
#include <support/ByteOrder.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>


#include "user_strings_beos.h"


// Mass debugging, overrides DEBUG in each file
#define USE_DEBUG_MODE 0

#if defined(__powerpc__) /* Native PowerPC */
# define WORDS_BIGENDIAN 1
# define SYSTEM_CLOBBERS_R2 1
# define DYNAMIC_ADDRESSING 1
#else /* Emulated PowerPC */
# undef WORDS_BIGENDIAN
# ifdef NATMEM_OFFSET
#  define DIRECT_ADDRESSING 1
# else
# define DYNAMIC_ADDRESSING 1
# endif
#endif

// Define for external components
#define SHEEPSHAVER 1
#define HAVE_SIGSEGV_SKIP_INSTRUCTION 1
#define CONFIGURE_TEST_SIGSEGV_RECOVERY

// High precision timing
#define PRECISE_TIMING 1
#define PRECISE_TIMING_BEOS 1

#define POWERPC_ROM 1

#if defined(__powerpc__) /* Native PowerPC */
# define ROM_IS_WRITE_PROTECTED 1
# define USE_SCRATCHMEM_SUBTERFUGE 0
# define PPC_ENABLE_JIT 0
#else /* Emulated PowerPC */
// Configure PowerPC emulator
# define PPC_REENTRANT_JIT 1
# define PPC_CHECK_INTERRUPTS 1
# define PPC_DECODE_CACHE 1
# define PPC_FLIGHT_RECORDER 1
# define PPC_PROFILE_COMPILE_TIME 0
# define PPC_PROFILE_GENERIC_CALLS 0
# define PPC_PROFILE_REGS_USE 0
# define PPC_ENABLE_FPU_EXCEPTIONS 0
# define PPC_ENABLE_JIT 0
# define KPX_MAX_CPUS 1
# if defined(__i386__) || defined(__x86_64__)
#  define DYNGEN_ASM_OPTS 1
# endif
#endif

#define VAL64(a) (a ## l)
#define UVAL64(a) (a ## ul)

// Byte swap functions
#define bswap_16 B_SWAP_INT16
#define bswap_32 B_SWAP_INT32
#define bswap_64 B_SWAP_INT64

#ifdef WORDS_BIGENDIAN
static inline uint16 tswap16(uint16 x) { return x; }
static inline uint32 tswap32(uint32 x) { return x; }
static inline uint64 tswap64(uint64 x) { return x; }
#else
static inline uint16 tswap16(uint16 x) { return bswap_16(x); }
static inline uint32 tswap32(uint32 x) { return bswap_32(x); }
static inline uint64 tswap64(uint64 x) { return bswap_64(x); }
#endif

// Time data type for Time Manager emulation
typedef bigtime_t tm_time_t;

// 64 bit file offsets
typedef off_t loff_t;

// Data types
typedef uint32 uintptr;
typedef int32 intptr;

// Timing functions
extern void Delay_usec(uint32 usec);
extern uint64 GetTicks_usec(void);


// Macro for calling MacOS routines
#if defined(__powerpc__)
#define CallMacOS(type, proc) (*(type)proc)()
#define CallMacOS1(type, proc, arg1) (*(type)proc)(arg1)
#define CallMacOS2(type, proc, arg1, arg2) (*(type)proc)(arg1, arg2)
#define CallMacOS3(type, proc, arg1, arg2, arg3) (*(type)proc)(arg1, arg2, arg3)
#define CallMacOS4(type, proc, arg1, arg2, arg3, arg4) (*(type)proc)(arg1, arg2, arg3, arg4)
#define CallMacOS5(type, proc, arg1, arg2, arg3, arg4, arg5) (*(type)proc)(arg1, arg2, arg3, arg4, arg5)
#define CallMacOS6(type, proc, arg1, arg2, arg3, arg4, arg5, arg6) (*(type)proc)(arg1, arg2, arg3, arg4, arg5, arg6)
#define CallMacOS7(type, proc, arg1, arg2, arg3, arg4, arg5, arg6, arg7) (*(type)proc)(arg1, arg2, arg3, arg4, arg5, arg6, arg7)
#else /* Emulated PowerPC */

// kpx_cpu MacOS routine wrappers
#define CallMacOS(type, tvect) call_macos((uintptr)tvect)
#define CallMacOS1(type, tvect, arg1) call_macos1((uintptr)tvect, (uintptr)arg1)
#define CallMacOS2(type, tvect, arg1, arg2) call_macos2((uintptr)tvect, (uintptr)arg1, (uintptr)arg2)
#define CallMacOS3(type, tvect, arg1, arg2, arg3) call_macos3((uintptr)tvect, (uintptr)arg1, (uintptr)arg2, (uintptr)arg3)
#define CallMacOS4(type, tvect, arg1, arg2, arg3, arg4) call_macos4((uintptr)tvect, (uintptr)arg1, (uintptr)arg2, (uintptr)arg3, (uintptr)arg4)
#define CallMacOS5(type, tvect, arg1, arg2, arg3, arg4, arg5) call_macos5((uintptr)tvect, (uintptr)arg1, (uintptr)arg2, (uintptr)arg3, (uintptr)arg4, (uintptr)arg5)
#define CallMacOS6(type, tvect, arg1, arg2, arg3, arg4, arg5, arg6) call_macos6((uintptr)tvect, (uintptr)arg1, (uintptr)arg2, (uintptr)arg3, (uintptr)arg4, (uintptr)arg5, (uintptr)arg6)
#define CallMacOS7(type, tvect, arg1, arg2, arg3, arg4, arg5, arg6, arg7) call_macos7((uintptr)tvect, (uintptr)arg1, (uintptr)arg2, (uintptr)arg3, (uintptr)arg4, (uintptr)arg5, (uintptr)arg6, (uintptr)arg7)

#ifdef __cplusplus
extern "C" {
#endif
//TODO: We should have a kpx_cpu header here, this is messy
extern uint32 call_macos(uint32 tvect);
extern uint32 call_macos1(uint32 tvect, uint32 arg1);
extern uint32 call_macos2(uint32 tvect, uint32 arg1, uint32 arg2);
extern uint32 call_macos3(uint32 tvect, uint32 arg1, uint32 arg2, uint32 arg3);
extern uint32 call_macos4(uint32 tvect, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4);
extern uint32 call_macos5(uint32 tvect, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4, uint32 arg5);
extern uint32 call_macos6(uint32 tvect, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4, uint32 arg5, uint32 arg6);
extern uint32 call_macos7(uint32 tvect, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4, uint32 arg5, uint32 arg6, uint32 arg7);
#ifdef __cplusplus
}
#endif

#endif /* !POWERPC */

#endif /* SYSDEPS_H */
