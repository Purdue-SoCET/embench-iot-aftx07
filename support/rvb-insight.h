# 0 "rvb-insight.h"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4

# 1 "/usr/include/stdc-predef.h" 3 4
/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */




/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */

/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
# 52 "/usr/include/stdc-predef.h" 3 4
/* wchar_t uses Unicode 10.0.0.  Version 10.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2017, fifth edition, plus
   the following additions from Amendment 1 to the fifth edition:
   - 56 emoji characters
   - 285 hentaigana
   - 3 additional Zanabazar Square characters */
# 0 "<command-line>" 2
# 1 "rvb-insight.h"



# 1 "../src/rvb-insight-print.h" 1

#define WRAP_CALL(...) \
    do { \
        uint32_t var__ = rvb_insight_wrap_begin(); \
        __VA_ARGS__; \
        rvb_insight_wrap_end(var__); \
    } while (0)

#define rvb_insight_print(...) \
    WRAP_CALL( \
        _Generic((0, ##__VA_ARGS__ +0), \
            default: rvb_insight_print_default, \
            uint32_t: rvb_insight_print_mask, \
            char *: rvb_insight_print_cfg \
        ) (__VA_ARGS__) \
    )

# 4 "../src/rvb-insight-print.h"
#include <stdint.h>

# 1 "../src/rvb-insight-internal.h" 1



#include <stdint.h>
#include <stdbool.h>

__attribute__((always_inline)) inline uint32_t __rvb_insight_wrap_begin(void);

__attribute__((always_inline)) inline void __rvb_insight_wrap_end(uint32_t old_mcountinhibit);

uint32_t __rvb_insight_read_register(uint32_t reg_num, bool high);
# 7 "../src/rvb-insight-print.h" 2
# 1 "../src/rvb-insight-cfgs.h" 1



# 1 "../src/../include/rvb-insight-constants.h" 1

#define MAX_CFGS 32

enum HPM_VALUES {
    HPM_CYCLE = 0,
    HPM_INSTRET = 2,
    HPM_I_MISS = 3,
    HPM_D_MISS = 4,
    HPM_I_HIT = 5,
    HPM_D_HIT = 6,
    HPM_iTLB_MISS = 7,
    HPM_dTLB_MISS = 8,
    HPM_iTLB_HIT = 9,
    HPM_dTLB_HIT = 10,
    HPM_BB_CYCLES = 11,
    HPM_BR_MISPRE = 12,
    HPM_BR_PRE = 13,
    HPM_FETCH_STALL = 14,
    HPM_EXEC_STALL = 15,
    HPM_MEM_STALL = 16
};

#define HPM_MASK(val) (1u << (val))

# 5 "../src/rvb-insight-cfgs.h" 2

#include <stdint.h>
#include <stddef.h>

struct RvbInsightConfig {
    const char *label;
    uint32_t hpm_enable;
};

extern struct RvbInsightConfig cfgs[32];

void rvb_insight_set_cfg(const char *label, uint32_t hpm_enable);

void rvb_insight_reset_cfgs();

uint32_t rvb_insight_get_cfg_mask(const char *label);

void rvb_insight_set_default_cfg(uint32_t hpm_enable);

size_t rvb_insight_search_cfgs(const char *label);
# 8 "../src/rvb-insight-print.h" 2

void rvb_insight_print_default(void);
void rvb_insight_print_mask(uint32_t hpm_enable);
void rvb_insight_print_cfg(const char *label);
# 5 "rvb-insight.h" 2

# 1 "../src/rvb-insight-cfgs.h" 1
# 7 "rvb-insight.h" 2

#ifndef MUTEX_H
#define MUTEX_H

// A simple mutex with a nonatomic lock and blocking lock/unlock methods. Make sure to zero
// initialize!
typedef struct {
    int lock;
} mutex_t;

// Atomically locks the mutex. Will spin until the lock is acquired.
void __attribute__((noinline)) mutex_lock(volatile mutex_t *m);
// Nonatomically unlocks the mutex. Should only be called if the mutex is held.
void __attribute__((noinline)) mutex_unlock(volatile mutex_t *m);

void atomic_set(void *ptr) {
    __asm__ volatile("li t0, 1\n"
                     "1:\n"
                     "lr.w t1, (%[addr])\n"
                     "bnez t1, 1b\n"
                     "sc.w t2, t0, (%[addr])\n"
                     "bnez t2, 1b\n"
                     :
                     : [addr] "r"(ptr)
                     : "t0", "t1", "t2");
}

void atomic_unset(void *ptr) {
    __asm__ volatile("1:\n"
                     "lr.w t0, (%[addr])\n"
                     "sc.w t0, zero, (%[addr])\n"
                     "bnez t0, 1b\n"
                     :
                     : [addr] "r"(ptr)
                     : "t0");
}

void mutex_lock(volatile mutex_t *m) {
    atomic_set((void *)&m->lock);
}

// This should only be called if we hold the lock, so it's okay to nonatomically unset it.
void mutex_unlock(volatile mutex_t *m) {
    m->lock = 0;
}
#endif
