/* BEEBS qrduino benchmark

   This version, copyright (C) 2014-2019 Embecosm Limited and University of
   Bristol

   Contributor James Pallister <james.pallister@bristol.ac.uk>
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later

   Original code from: https://github.com/tz1/qrduino */

#include "qrencode.h"
#include "support.h"
#include "rvb-insight.h"

#include <string.h>

/* This scale factor will be changed to equalise the runtime of the
   benchmarks. */
#define LOCAL_SCALE_FACTOR 5

/* BEEBS heap is just an array */

#define HEAP_SIZE 8192
static char heap[HEAP_SIZE];

static const char *encode;
static int size;

static int benchmark_body(int rpt);

void warm_caches(int heat) {
    int res = benchmark_body(heat);

    return;
}

int benchmark(void) {
    return benchmark_body(LOCAL_SCALE_FACTOR * CPU_MHZ);
}

static int __attribute__((noinline)) benchmark_body(int rpt) {
    static const char *in_encode = "http://www.mageec.com";
    int i;

    rvb_insight_set_default_cfg(0x7);
    rvb_insight_set_cfg("test", 0x5);

    for (i = 0; i < rpt; i++) {
        encode = in_encode;
        size = 22;
        init_heap_beebs((void *)heap, HEAP_SIZE);

        initeccsize(1, size);

        memcpy(strinbuf, encode, size);

        initframe();
        qrencode();
        freeframe();
        freeecc();

        rvb_insight_print();
        rvb_insight_print("test");
        // rvb_insight_print(0x3 | HPM_MASK(HPM_iTLB_HIT)); // broken for some reason
        rvb_insight_print(0x3);
    }

    return 0;
}

void initialise_benchmark() {}

int verify_benchmark(int unused) {
    unsigned char expected[22] = {254, 101, 63, 128, 130, 110, 160, 128, 186, 65,  46,
                                  128, 186, 38, 46,  128, 186, 9,   174, 128, 130, 20};

    return (0 == memcmp(strinbuf, expected, 22 * sizeof(strinbuf[0]))) &&
           check_heap_beebs((void *)heap);
}

/*
   Local Variables:
   mode: C
   c-file-style: "gnu"
   End:
*/
