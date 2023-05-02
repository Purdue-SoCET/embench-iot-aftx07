#!/bin/bash
# Assumes that these envvars are set:
#   $AFTX07_ROOT: Path to the top directory of the AFTx07 core

set -e

if [ -z ${AFTX07_ROOT} ]; then 
    echo "\$AFTX07_ROOT is unset!"
    return
fi

# Declare different cache properties
CACHE_SIZES=(1024 2048 4096)
BLOCK_SIZES=(2 4)
ASSOCIATIVITY=(1 2)

echo "Building benchmarks..."
build_all.py --arch=riscv32 --chip=aftx07 --board=aftx07 --builddir bd_aftx07 --clean

for i_cache_size in "${CACHE_SIZES[@]}"; do
    for d_cache_size in "${CACHE_SIZES[@]}"; do
        for block_size in "${BLOCK_SIZES[@]}"; do
            for associativity in "${ASSOCIATIVITY[@]}"; do
                echo "Replacing cache parameters..."
                # Replace cache parameters
                sed -i "s/dcache_size.*/dcache_size : $d_cache_size/g" $AFTX07_ROOT/aftx07.rvbcfg.yml
                sed -i "s/icache_size.*/icache_size : $i_cache_size/g" $AFTX07_ROOT/aftx07.rvbcfg.yml
                sed -i "s/dcache_block_size.*/dcache_block_size : $block_size/g" $AFTX07_ROOT/aftx07.rvbcfg.yml
                sed -i "s/icache_block_size.*/icache_block_size : $block_size/g" $AFTX07_ROOT/aftx07.rvbcfg.yml
                sed -i "s/dcache_assoc.*/dcache_assoc : $associativity/g" $AFTX07_ROOT/aftx07.rvbcfg.yml
                sed -i "s/icache_assoc.*/icache_assoc : $associativity/g" $AFTX07_ROOT/aftx07.rvbcfg.yml

                # Rebuild AFTx07 core
                pushd $AFTX07_ROOT
                ./build.sh
                popd

                # Run benchmarks
                echo "Running benchmarks..."
                benchmark_speed.py --target-module run_aftx07 --timeout=360000 --builddir bd_aftx07 --sim-parallel > out_aftx07_i${i_cache_size}d${d_cache_size}b${block_size}a${associativity}
            done
        done
    done
done
