import yaml
import argparse
import os
import shutil
import subprocess
import sys
import benchmark_speed

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
    # parser.add_argument(
    #     '--logdir',
    #     type=str,
    #     default='logs',
    #     help='Directory in which to store logs',
    # )
    # parser.add_argument(
    #     '--baselinedir',
    #     type=str,
    #     default='baseline-data',
    #     help='Directory which contains baseline data',
    # )
    # parser.add_argument(
    #     '--absolute',
    #     action='store_true',
    #     help='Specify to show absolute results',
    # )
    # parser.add_argument(
    #     '--relative',
    #     dest='absolute',
    #     action='store_false',
    #     help='Specify to show relative results (the default)',
    # )
    # parser.add_argument(
    #     '--json-output',
    #     dest='output_format',
    #     action='store_const',
    #     const=output_format.JSON,
    #     help='Specify to output in JSON format',
    # )
    # parser.add_argument(
    #     '--text-output',
    #     dest='output_format',
    #     action='store_const',
    #     const=output_format.TEXT,
    #     help='Specify to output as plain text (the default)',
    # )
    # parser.add_argument(
    #     '--baseline-output',
    #     dest='output_format',
    #     action='store_const',
    #     const=output_format.BASELINE,
    #     help='Specify to output in a format suitable for use as a baseline'
    # )
    # parser.add_argument(
    #     '--json-comma',
    #     action='store_true',
    #     help='Specify to append a comma to the JSON output',
    # )
    # parser.add_argument(
    #     '--no-json-comma',
    #     dest='json_comma',
    #     action='store_false',
    #     help='Specify to not append a comma to the JSON output',
    # )
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
    # parser.add_argument(
    #     '--sim-parallel',
    #     action='store_true',
    #     default=False,
    #     help='Launch all benchmarks in parallel'
    # )
    # parser.add_argument(
    #     '--sim-serial',
    #     dest='sim_parallel',
    #     action='store_false',
    #     help='Launch all benchmarks in series (the default)'
    # )
    parser.add_argument(
        '--aft-toplevel',
        type=str,
        required=True,
        help='Path to the AFT'
    )
    parser.add_argument(
        '--aft-config',
        type=str,
        required=True,
        help='RISC-V core config file name'
    )

    return parser.parse_known_args()

def build_run_command(args):
    run_command = ['python', 'benchmark_speed.py']

    for arg, value in vars(args).items():
        if arg != "aft_toplevel" and arg != "aft_config" and value != None:
            run_command.append("--" + arg.replace('_', '-'))
            run_command.append(str(value))

    run_command.append("--sim-parallel")

    return run_command

def edit_config(configdir, cache, block, assoc):
    print(f"Editing config file at {configdir}")
    
    config = load_configuration(configdir)

    microarch_params = config['microarch_params']

    # change dcache configs
    microarch_params['dcache_size']       = cache
    microarch_params['dcache_block_size'] = block
    microarch_params['dcache_assoc']      = assoc

    # change icache configs
    microarch_params['icache_size']       = cache
    microarch_params['icache_block_size'] = block
    microarch_params['icache_assoc']      = assoc

    config['microarch_params'] = microarch_params

    save_configuration(configdir, config)

    return

def build_core(aft_toplevel):
    print("Building core...")
    try:
        res = subprocess.run(
            ['./build.sh'],
            stdout=subprocess.PIPE, 
            stderr=subprocess.PIPE,
            cwd=aft_toplevel
        )
        print("Success!")
    except:
        print("\nFailed!")
    

def main():
    """Main program testing benchmark speeds with variable L1 caches for the AFT"""

    CACHE_SIZES = [1024, 2048]
    BLOCK_SIZES = [2, 4]
    ASSOC_SIZES = [1, 2]

    args, _ = parse_args()

    run_command = build_run_command(args)

    # edit_config(args.aft_toplevel + "/" + args.aft_config, 1024, 2, 1)

    # build_core(args.aft_toplevel)

    for assoc in ASSOC_SIZES:
        for cache in CACHE_SIZES:
            for block in BLOCK_SIZES:
                print("\n----------------------------------------\n")
                edit_config(args.aft_toplevel + "/" + args.aft_config, cache, block, assoc)

                build_core(args.aft_toplevel)

                try:
                    print(f"Running embench for cache size {cache}, block size {block}, and associativity {assoc}")
                    res = subprocess.run(
                        run_command,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE
                    )
                except:
                    print("\nError with running benchmark_speed.py, check log")

                output_dir = f"logs/benchmark_speed_c{cache}_b{block}_a{assoc}.log"
                with open(output_dir, 'w') as f:
                    f.write(res.stdout.decode('utf-8'))
                    
                


    return

if __name__ == "__main__":
  sys.exit(main())