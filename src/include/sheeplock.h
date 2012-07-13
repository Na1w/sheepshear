/*
 *  sheeptypes.h - simple spinlock wrappers
 *
 *  SheepShear, 2012 Alexander von Gluck
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
#ifndef __SHEEPLOCKS_H
#define __SHEEPLOCKS_H


#define HAVE_SPINLOCKS 1

 
typedef volatile int spinlock_t;

static const spinlock_t SPIN_LOCK_UNLOCKED = 0;

#define atomic_cmp_set(a,b,c) __sync_bool_compare_and_swap(a,b,c)
#define atomic_add_fetch(a,b,c) __sync_add_and_fetch(a,b,c)
#define HAVE_SPINLOCKS 1


class SpinLock
{
public:
    SpinLock() : spinLock(SPIN_LOCK_UNLOCKED) {}
    void Lock() { while (!atomic_cmp_set(&spinLock, 0, 1)); }
    void Unlock() { spinLock = 0; }
    int TryLock() { return atomic_cmp_set(&spinLock, 0, 1); }
private:
    spinlock_t spinLock;
};


#endif /* __SHEEPLOCKS_H */