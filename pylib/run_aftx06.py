#!/usr/bin/env python3

# Python module to run programs on AFTx06

# Contributor: Grant Goldenberg <ggoldenb@purdue.edu>
#
# This file is not part of Embench.

"""
Embench module to run benchmark programs.

This version is suitable for the AFTx06 simulator.
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

from embench_core import log

cpu_mhz = 1
meminit_elf = ['riscv64-unknown-elf-objcopy', '-O', 'binary']
bd_src = "/home/asicfab/a/socet21/embench-iot/bd/src/"
cp = ['cp', 'meminit.bin', '/home/asicfab/a/socet21/AFTx06/meminit.bin']
rm = ['rm', '/home/asicfab/a/socet21/AFTx06/meminit.bin']
aftx06_root = "/home/asicfab/a/socet21/AFTx06"

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
	subprocess.run(cp, cwd=bd_src_cpy)

	print(f"Running {bench}")
	return ['make', 'no_gui', '-C', '/home/asicfab/a/socet21/AFTx06']

def decode_results(stdout_str, stderr_str):
	matchval = re.findall(r"^[0-9]+", stdout_str, re.MULTILINE)
	elapsed_cycles = matchval[1]
	
	subprocess.run(rm, cwd=aftx06_root)

	global cpu_mhz	
	print(f"Speed: {int(elapsed_cycles) / cpu_mhz / 1000}")
	print("Returning result.")

	return int(elapsed_cycles) / cpu_mhz / 1000

