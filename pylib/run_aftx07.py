#!/usr/bin/env python3

# Python module to run programs on AFTx07

# Contributor: Devin Singh <singh956@purdue.edu>
#
# This file is not part of Embench.

"""
Embench module to run benchmark programs.

This version is suitable for the AFTx07 simulator.
"""

__all__ = [
    'get_target_args',
    'build_benchmark_cmd',
    'decode_results',
]

import argparse
import copy
import re
import subprocess
import os

from embench_core import log

root = os.path.join(os.path.dirname(__file__), "..")

aftx07_root = os.getenv('AFTX07_ROOT')
cpu_mhz = 1
meminit_elf = ['riscv64-unknown-elf-objcopy', '-O', 'binary']
bd_src = os.path.join(root, "bd/src/")
cp = ['cp', 'meminit.bin', aftx07_root + '/meminit.bin']

def get_target_args(remnant):
    """Parse left over arguments"""
    parser = argparse.ArgumentParser(description='Get target specific args')

    parser.add_argument(
        '--cpu-mhz',
        type=int,
        default=1,
        help='Processor clock speed in MHz'
    )

    return parser.parse_args(remnant)

def build_benchmark_cmd(bench, args):
    """Construct the command to run the benchmark. "args" is a 
       namespace with target specific arguments"""

    meminit_elf_cpy = copy.deepcopy(meminit_elf)
    meminit_elf_cpy.append(bench)
    meminit_elf_cpy.append("meminit.bin")
    bd_src_cpy = copy.deepcopy(bd_src)
    bd_src_cpy = bd_src_cpy + bench
    subprocess.run(meminit_elf_cpy, cwd=bd_src_cpy)
    subprocess.run(['cp', bench, aftx07_root + '/meminit.bin'], cwd=bd_src_cpy)

    print(f"Running {bench}")
    return [aftx07_root + '/aft_out/sim-verilator/Vaftx07']

def decode_results(stdout_str, stderr_str):
    matchval = re.findall(r"[0-9]+", stdout_str, re.MULTILINE)
    elapsed_cycles = int(matchval[1])
    elapsed_instrs = int(matchval[2])

    global cpu_mhz
    print(f'Speed: {elapsed_cycles / cpu_mhz / 1000}')
    print(f'Instructions: {elapsed_instrs}')
    print('Returning result.')

    return elapsed_cycles / cpu_mhz / 1000

