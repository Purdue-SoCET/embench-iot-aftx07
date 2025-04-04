import yaml
import argparse
import os
import shutil
import subprocess
import sys
import benchmark_speed
import time

from embench_core import check_python_version
from embench_core import log
from embench_core import gp
from embench_core import setup_logging
from embench_core import log_args
from embench_core import find_benchmarks
from embench_core import log_benchmarks
from embench_core import embench_stats
from embench_core import output_format

def load_configuration(file_name):
    with open(file_name, 'r') as f:
        try:
            config = yaml.full_load(f)
        except yaml.parser.ParserError:
            sys.exit('Parse of '+ file_name + ' failed. Please check yml syntax')
    return config

def save_configuration(file_name, config):
    with open(file_name, 'w') as f:
        try:
            yaml.dump(config, f, default_flow_style=False, sort_keys=False)
        except yaml.parser.ParserError as y:
            print(y)
            sys.exit('Write of '+ file_name + ' failed.')

def parse_args():
    """Build a parser for all the arguments"""
    parser = argparse.ArgumentParser(description='Compute the size benchmark')

    parser.add_argument(
        '--builddir',
        type=str,
        default='bd',
        help='Directory holding all the binaries',
    )
    parser.add_argument(
        '--target-module',
        type=str,
        required=True,
        help='Python module with routines to run benchmarks',
    )
    parser.add_argument(
        '--timeout',
        type=int,
        default=30,
        help='Timeout used for running each benchmark program'
    )
    parser.add_argument(
        '--rvb-core',
        type=str,
        required=True,
        help='Path to the RISCV core'
    )
    parser.add_argument(
        '--rvb-config',
        type=str,
        required=True,
        help='RISC-V core config file name'
    )

    return parser.parse_known_args()

def build_run_command(args):
    run_command = ['python', 'benchmark_speed.py']

    for arg, value in vars(args).items():
        if arg != "rvb-core" and arg != "rvb-config" and value != None:
            run_command.append("--" + arg.replace('_', '-'))
            run_command.append(str(value))

    run_command.append("--sim-parallel")

    return run_command

def edit_config(configdir, tlb_entries):
    print(f"Editing config file at {configdir}")
    
    config = load_configuration(configdir)

    microarch_params = config['microarch_params']

    # change number of tlb entries
    microarch_params['tlb_entries'] = tlb_entries

    config['microarch_params'] = microarch_params

    save_configuration(configdir, config)

    return

def build_core(rvb_toplevel):
    print("Building core...")
    res = None
    try:
        res = subprocess.run(
            ['make', 'clean'],
            stdout=subprocess.PIPE, 
            stderr=subprocess.PIPE,
            cwd=rvb_toplevel
        )
        res = subprocess.run(
            ['make', 'verilate'],
            stdout=subprocess.PIPE, 
            stderr=subprocess.PIPE,
            cwd=rvb_toplevel
        )
        time.sleep(5) # trying something...
        print(res)
        print("Success!")
    except:
        print("\nFailed!")
    

def main():
    """Main program testing benchmark speeds with variable TLB entries for RISCVBusiness"""

    TLB_ENTRIES = [1, 2, 4, 8, 16, 32, 64]

    args, _ = parse_args()

    run_command = build_run_command(args)

    rvb_core = os.getenv('RISCV_CORE')
    rvb_root = rvb_core.replace("/rvb_out/sim-verilator/Vtop_core", "")

    for tlb_entries in TLB_ENTRIES:
        print("\n----------------------------------------\n")
        edit_config(rvb_root + "/" + args.rvb_config, tlb_entries)

        build_core(rvb_root)

        try:
            print(f"Running embench for {tlb_entries} TLB entries") # sorry, grammar will be wrong for 1 :(
            res = subprocess.run(
                run_command,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
        except:
            print("\nError with running benchmark_speed.py, check log")

        output_dir = f"logs/embench_rvb_supervisor_tlb{tlb_entries:02d}.log"
        with open(output_dir, 'w') as f:
            f.write(res.stdout.decode('utf-8'))

        break


    return

if __name__ == "__main__":
    sys.exit(main())