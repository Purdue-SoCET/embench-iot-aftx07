/* Board support for generic AFTx07 board */

#include "format.h"
#include <stdint.h>
#include <support.h>

// Generates a function that returns a u64 from a CSR
#define GET_CSR(type)                                                          \
    static inline uint32_t get_##type() {                                      \
        uint32_t lo;                                                           \
        __asm__ volatile("csrr %0, " #type : "=r"(lo));                        \
        return lo;                                                             \
    }

static uint32_t start_cycles;
static uint32_t start_instrs;

// Generate functions to get cycle counter and number of instructions retired
GET_CSR(mcycle)
GET_CSR(minstret)

void __attribute__((interrupt)) __attribute__((aligned(4))) handler() {
    uint32_t mepc_value;
    uint32_t mcause_value;
    asm volatile("csrr %0, mepc" : "=r"(mepc_value));
    asm volatile("csrr %0, mcause" : "=r"(mcause_value));
    print("mepc: %d\n", mepc_value);
    print("mcause: %d\n", mcause_value);
    while (1) {}
}

void initialise_board(void) {
    uint32_t mtvec_value = (uint32_t)handler;
    uint32_t mstatus_value = 0x8;
    asm volatile("csrw mstatus, %0" : : "r"(mstatus_value));
    asm volatile("csrw mtvec, %0" : : "r"(mtvec_value));
    print("Initializing board!n");
}

void print_verify_benchmark(int res) {
    print("Benchmark Return: %d", res);
}

void __attribute__((noinline)) __attribute__((externally_visible))
start_trigger(void) {
    print("Start trigger!\n");
    start_cycles = get_mcycle();
    start_instrs = get_minstret();
}

void __attribute__((noinline)) __attribute__((externally_visible))
stop_trigger(void) {
    volatile uint32_t *ptr = (uint32_t *)0x1000;
    uint32_t end_cycles, end_instrs;
    end_cycles = get_mcycle();
    end_instrs = get_minstret();
    print("Total cycles: %u\n", end_cycles - start_cycles);
    print("Total Instructions: %u\n", end_instrs - start_instrs);
}
