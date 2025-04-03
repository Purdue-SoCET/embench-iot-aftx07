/* Board support for generic AFTx07 board */

#include "format.h"
#include <stdint.h>
#include <support.h>

// Generates a function that returns a u64 from a CSR
#define GET_CSR(type)                                                                              \
    static inline uint32_t get_##type() {                                                          \
        uint32_t lo;                                                                               \
        __asm__ volatile("csrr %0, " #type : "=r"(lo));                                            \
        return lo;                                                                                 \
    }

static uint32_t start_cycles;
static uint32_t start_instrs;

// Generate functions to get cycle counter and number of instructions retired
GET_CSR(mcycle)
GET_CSR(minstret)

void __attribute__((interrupt)) __attribute__((aligned(4))) handler() {
    uint32_t sepc_value;
    uint32_t scause_value;
    asm volatile("csrr %0, sepc" : "=r"(sepc_value));
    asm volatile("csrr %0, scause" : "=r"(scause_value));
    print("sepc: %d\n", sepc_value);
    print("scause: %d\n", scause_value);
    while (1) {}
}

void initialise_board(void) {
}

void print_verify_benchmark(int res) {
}

void __attribute__((noinline)) __attribute__((externally_visible)) start_trigger(void) {
}

void __attribute__((noinline)) __attribute__((externally_visible)) stop_trigger(void) {
}
