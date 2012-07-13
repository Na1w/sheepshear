/*
 *  sysdeps.h - System dependent definitions for Linux
 *
 *  SheepShear, 2012 Alexander von Gluck IV
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
#ifndef SYSDEPS_H
#define SYSDEPS_H


// Mass debugging, overrides DEBUG in each file
#define USE_DEBUG_MODE 0

#ifndef __STDC__
#error "Your compiler is not ANSI. Get a real one."
#endif

#include "config.h"
#include "sheeptypes.h"
 
#ifndef STDC_HEADERS
#error "You don't have ANSI C header files."
#endif

#include "user_strings_unix.h"

#ifdef HAVE_UNISTD_H
# include <sys/types.h>
# include <unistd.h>
#endif

#include <netinet/in.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
 
#ifdef HAVE_PTHREADS
# include <pthread.h>
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef __MACH__
#include <mach/mach_types.h>
#endif

// Fix offsetof() on FreeBSD and GCC >= 3.4
#if defined(__FreeBSD__) && defined(__cplusplus)
#undef offsetof
/* The cast to "char &" below avoids problems with user-defined
   "operator &", which can appear in a POD type.  */
#define offsetof(TYPE, MEMBER)                          \
  (__offsetof__ (reinterpret_cast <size_t>              \
                 (&reinterpret_cast <char &>            \
                  (static_cast<TYPE *> (0)->MEMBER))))
#endif

// Define for external components
#define SHEEPSHAVER 1

// Always use Real Addressing mode on native architectures
// Otherwise, use Direct Addressing mode if NATMEM_OFFSET is set
#if defined(__powerpc__) /* Native PowerPC */
# define REAL_ADDRESSING 1
# include "ppc_asm.tmpl"
#elif defined(NATMEM_OFFSET)
# define DIRECT_ADDRESSING 1
#else /* Emulated PowerPC */
#define REAL_ADDRESSING 1
#endif

// Always use the complete non-stubs Ethernet driver
#define USE_ETHER_FULL_DRIVER 1

#define POWERPC_ROM 1

#if defined(__powerpc__) /* Native PowerPC */
// Mac ROM is write protected
#define ROM_IS_WRITE_PROTECTED 1
#define USE_SCRATCHMEM_SUBTERFUGE 0
#else /* Emulated PowerPC */
// Mac ROM is write protected when banked memory is used
#if REAL_ADDRESSING || DIRECT_ADDRESSING
# define ROM_IS_WRITE_PROTECTED 0
# define USE_SCRATCHMEM_SUBTERFUGE 1
#else
# define ROM_IS_WRITE_PROTECTED 1
#endif
// Configure PowerPC emulator
#define PPC_REENTRANT_JIT 1
#define PPC_CHECK_INTERRUPTS 1
#define PPC_DECODE_CACHE 1
#define PPC_FLIGHT_RECORDER 1
#define PPC_PROFILE_COMPILE_TIME 0
#define PPC_PROFILE_GENERIC_CALLS 0
#define PPC_PROFILE_REGS_USE 0
#define PPC_ENABLE_FPU_EXCEPTIONS 0
#define KPX_MAX_CPUS 1
#if ENABLE_DYNGEN
#define PPC_ENABLE_JIT 1
#endif
#if defined(__i386__) || defined(__x86_64__)
#define DYNGEN_ASM_OPTS 1
#endif
#endif


// Define if the host processor supports fast unaligned load/stores
#if defined __i386__ || defined __x86_64__
#define UNALIGNED_PROFITABLE 1
#endif


/**
 *		Helper functions to byteswap data
 **/
#if defined(__GNUC__)
#if defined(__x86_64__) || defined(__i386__)
// Linux/AMD64 currently has no asm optimized bswap_32() in <byteswap.h>
#define opt_bswap_32 do_opt_bswap_32
static inline uint32 do_opt_bswap_32(uint32 x)
{
  uint32 v;
  __asm__ __volatile__ ("bswap %0" : "=r" (v) : "0" (x));
  return v;
}
#endif
#endif

#ifdef HAVE_BYTESWAP_H
#include <byteswap.h>
#endif

#ifdef  opt_bswap_16
#undef  bswap_16
#define bswap_16 opt_bswap_16
#endif
#ifndef bswap_16
#define bswap_16 generic_bswap_16
#endif

static inline uint16 generic_bswap_16(uint16 x)
{
  return ((x & 0xff) << 8) | ((x >> 8) & 0xff);
}

#ifdef  opt_bswap_32
#undef  bswap_32
#define bswap_32 opt_bswap_32
#endif
#ifndef bswap_32
#define bswap_32 generic_bswap_32
#endif

static inline uint32 generic_bswap_32(uint32 x)
{
  return (((x & 0xff000000) >> 24) |
		  ((x & 0x00ff0000) >>  8) |
		  ((x & 0x0000ff00) <<  8) |
		  ((x & 0x000000ff) << 24) );
}

#if defined(__i386__)
#define opt_bswap_64 do_opt_bswap_64
static inline uint64 do_opt_bswap_64(uint64 x)
{
  return (bswap_32(x >> 32) | (((uint64)bswap_32((uint32)x)) << 32));
}
#endif

#ifdef  opt_bswap_64
#undef  bswap_64
#define bswap_64 opt_bswap_64
#endif
#ifndef bswap_64
#define bswap_64 generic_bswap_64
#endif

static inline uint64 generic_bswap_64(uint64 x)
{
  return (((x & UVAL64(0xff00000000000000)) >> 56) |
		  ((x & UVAL64(0x00ff000000000000)) >> 40) |
		  ((x & UVAL64(0x0000ff0000000000)) >> 24) |
		  ((x & UVAL64(0x000000ff00000000)) >>  8) |
		  ((x & UVAL64(0x00000000ff000000)) <<  8) |
		  ((x & UVAL64(0x0000000000ff0000)) << 24) |
		  ((x & UVAL64(0x000000000000ff00)) << 40) |
		  ((x & UVAL64(0x00000000000000ff)) << 56) );
}

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
#ifdef HAVE_CLOCK_GETTIME
typedef struct timespec tm_time_t;
#elif defined(__MACH__)
typedef mach_timespec_t tm_time_t;
#else
typedef struct timeval tm_time_t;
#endif

/* Define codes for all the float formats that we know of.
 * Though we only handle IEEE format.  */
#define UNKNOWN_FLOAT_FORMAT 0
#define IEEE_FLOAT_FORMAT 1
#define VAX_FLOAT_FORMAT 2
#define IBM_FLOAT_FORMAT 3
#define C4X_FLOAT_FORMAT 4

// High-precision timing
#if defined(HAVE_PTHREADS) && defined(HAVE_CLOCK_NANOSLEEP)
#define PRECISE_TIMING 1
#define PRECISE_TIMING_POSIX 1
#elif defined(HAVE_PTHREADS) && defined(__MACH__)
#define PRECISE_TIMING 1
#define PRECISE_TIMING_MACH 1
#endif

// Timing functions
extern uint64 GetTicks_usec(void);
extern void Delay_usec(uint32 usec);

#ifdef HAVE_PTHREADS
// Setup pthread attributes
extern void Set_pthread_attr(pthread_attr_t *attr, int priority);
#endif

// Various definitions
typedef struct rgb_color {
	uint8		red;
	uint8		green;
	uint8		blue;
	uint8		alpha;
} rgb_color;


// Macro for calling MacOS routines
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

#endif
