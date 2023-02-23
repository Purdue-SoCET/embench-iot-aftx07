#!/bin/sh

build_all.py --arch=riscv32 --chip=aftx07 --board=aftx07 --builddir bd_aftx07 --clean
benchmark_speed.py --target-module run_aftx07 --timeout=360000 --builddir bd_aftx07 > out_aftx07 2>&1 &
