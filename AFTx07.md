# Compiling and running benchmarks for AFTx07

The benchmarks take about 8 minutes to run on asicfab.

Make sure that you build AFTx07 with at least 64K of memory.

```git
--- a/top_level/src/aftx07.sv
+++ b/top_level/src/aftx07.sv
@@ -29,7 +29,8 @@ module aftx07 #(
     //`ifndef SYNTHESIS

     localparam int K = 1024;
-    localparam int MEM_SIZE = 8*K;
+    localparam int M = K * K;
+    localparam int MEM_SIZE = 64 * K;
     localparam int MEM_N_WORDS = MEM_SIZE / 4;

     //localparam int PRINTER_APB_IDX  = 0;
```

```sh
build_all.py --arch=riscv32 --chip=aftx07 --board=aftx07 --clean
benchmark_speed.py --target-module run_aftx07 --timeout=3600 --sim-parallel
```
