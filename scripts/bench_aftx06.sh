#!/bin/sh

build_all.py --arch=riscv32 --chip=aftx06 --board=aftx06 --builddir bd_aftx06 --clean
benchmark_speed.py --target-module run_aftx06_verilator --timeout=360000 --builddir bd_aftx06 > out_aftx06 2>&1 &
