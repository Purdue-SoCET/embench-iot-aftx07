#!/bin/sh

build_all.py --arch=riscv32 --chip=aftx07 --board=aftx07 --builddir bd_aftx07_trace --clean
benchmark_speed.py --target-module run_aftx07_trace --timeout=360000 --builddir bd_aftx07_trace > out_aftx07_trace 2>&1 &
