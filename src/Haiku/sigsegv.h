/*
 *  sigsegv.h - System dependent signal wrappers for Haiku
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
#ifndef _SIGSEGV_H
#define _SIGSEGV_H


#include <KernelExport.h>


#define HAVE_SPINLOCKS 1


// Signal handling
typedef char *sigsegv_address_t;

struct sigsegv_info_t {
	sigsegv_address_t addr;
	sigsegv_address_t pc;
};

// SIGSEGV handler return state
enum sigsegv_return_t {
	SIGSEGV_RETURN_SUCCESS,
	SIGSEGV_RETURN_FAILURE,
	SIGSEGV_RETURN_SKIP_INSTRUCTION
};

typedef volatile long spinlock_t;

static const spinlock_t SPIN_LOCK_UNLOCKED = 0;


static inline void spin_lock(spinlock_t *lock)
{
	acquire_spinlock(lock);
}


static inline void
spin_unlock(spinlock_t *lock)
{
	release_spinlock(lock);
}


static inline int
spin_trylock(spinlock_t *lock)
{
	return 1;
}


// Return the address of the invalid memory reference
static sigsegv_address_t
sigsegv_get_fault_address(sigsegv_info_t *SIP)
{
	return SIP->addr;
}


// Return the address of the instruction that caused the fault, or
// SIGSEGV_INVALID_ADDRESS if we could not retrieve this information
static sigsegv_address_t
sigsegv_get_fault_instruction_address(sigsegv_info_t *SIP)
{
	return SIP->pc;
}


#endif /* _SIGSEGV_H */
