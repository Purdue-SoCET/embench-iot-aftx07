/* Common main.c for the benchmarks

   Copyright (C) 2014 Embecosm Limited and University of Bristol
   Copyright (C) 2018-2019 Embecosm Limited

   Contributor: James Pallister <james.pallister@bristol.ac.uk>
   Contributor: Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later */

#include "support.h"

int __attribute__((used))
main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    int i;
    volatile int result;
    int correct;

    print("starting benchmark\n");
    initialise_board();
    initialise_benchmark();
    warm_caches(WARMUP_HEAT);

    start_trigger();
    result = benchmark();
    stop_trigger();
    print("benchmark completed");

    /* bmarks that use arrays will check a global array rather than int result */

    correct = verify_benchmark(result);
    if (!correct) {
        print("BENCHMARK FAILED\n");
    } else {
        print("BENCHMARK PASSED\n");
    }

    return (!correct);

} /* main () */

/*
   Local Variables:
   mode: C
   c-file-style: "gnu"
   End:
*/
