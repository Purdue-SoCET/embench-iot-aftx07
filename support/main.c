/* Common main.c for the benchmarks

   Copyright (C) 2014 Embecosm Limited and University of Bristol
   Copyright (C) 2018-2019 Embecosm Limited

   Contributor: James Pallister <james.pallister@bristol.ac.uk>
   Contributor: Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later */

#include "support.h"
#include <stdint.h>

int __attribute__((used))
main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    int i;
    volatile int result;
    int correct;

    // No boards to initialize

    initialise_benchmark();
    // warm_caches(WARMUP_HEAT);

    result = benchmark(); // Warmup

    uint32_t start_cycles, end_cycles, total_cycles;
    uint32_t start_instrs, end_instrs, total_instrs;
    asm volatile ("csrr %0, cycle" : "=r"(start_cycles)); // start_trigger()
    asm volatile ("csrr %0, instret" : "=r"(start_instrs)); // start_trigger()
    result = benchmark();
    asm volatile ("csrr %0, cycle" : "=r"(end_cycles)); // stop_trigger()
    asm volatile ("csrr %0, instret" : "=r"(end_instrs)); // stop_trigger()

    total_cycles = end_cycles - start_cycles;
    total_instrs = end_instrs - start_instrs;

    /* bmarks that use arrays will check a global array rather than int result */

    correct = verify_benchmark(result);

    asm volatile ("mv a0, %0" : : "r" (correct ? total_cycles : 0)); // only way we can return this
    asm volatile ("mv a1, %0" : : "r" (correct ? total_instrs : 0)); // only way we can return this
    asm volatile ("ecall");

    return (!correct); // should be unreachable

} /* main () */

/*
   Local Variables:
   mode: C
   c-file-style: "gnu"
   End:
*/
