/* Board support for generic AFTx06 board */

#include <stdint.h>
#include <support.h>

static uint64_t start_cycles;
static uint64_t start_instrs;

typedef enum {
	CYCLE,
	INSTR
} csr_t;

static inline uint64_t
get_csr(csr_t sel)
{
	uint32_t lo, hi;
	if (sel == CYCLE) {
		__asm__ volatile("csrr %0,mcycle" : "=r" (lo));
		__asm__ volatile("csrr %0,mcycleh" : "=r" (hi));
	} else if (sel == INSTR) {
		__asm__ volatile("csrr %0,minstret" : "=r" (lo));
		__asm__ volatile("csrr %0,minstreth" : "=r" (hi));
	}
	return ((uint64_t) hi << 32) | ((uint64_t) lo);
}

void 
creverse(char *str, size_t len)
{
	char temp;
	size_t loop_idx;

	for (loop_idx = 0; loop_idx < len / 2; loop_idx++) {
		temp = str[loop_idx];
		str[loop_idx] = str[len - loop_idx - 1];
		str[len - loop_idx - 1] = temp;
	}
}

char *
citoa(uint64_t val, char *val_str)
{
	size_t arr_idx;
	
	if (val == 0) {
		val_str[0] = '0';
		val_str[1] = '\0';
	} else {
		for (arr_idx = 0; val != 0; arr_idx++) {
			val_str[arr_idx] = ((val % 10) > 9) ? ((val % 10) - 10) + 'a' : (val % 10) + '0';
			val /= 10;
		}
		val_str[arr_idx] = '\0';
		creverse(val_str, arr_idx);
	}
	return val_str;
}

int numPlaces (int n) {
    int r = 1;
    if (n < 0) n = (n == -2147483648) ? 2147483647: -n;
    while (n > 9) {
        n /= 10;
        r++;
    }
    return r;
}

int reversDigits(int num)
{
    int rev_num = 0;
    while (num > 0) {
        rev_num = rev_num * 10 + num % 10;
        num = num / 10;
    }
    return rev_num;
}

static inline void
cprintf(char * str) {
	size_t loop_idx;
	volatile char *magic = 0x20000;
	for (loop_idx = 0; str[loop_idx] != '\0'; loop_idx++) {
		*magic = str[loop_idx];
	}
	*magic = '\n';
}

static inline void
cprintfd(char * str) {
	size_t loop_idx;
	volatile char *magic = 0x20000;
	for (loop_idx = 0; str[loop_idx] != '\0'; loop_idx++) {
		*magic = str[loop_idx];
	}
}

static inline void
cprintfi(int num) {
	char numC;
	volatile char *magic = 0x20000;
	uint64_t rev = reversDigits(num);
	int np = numPlaces(num);
	for(int i = 0; i < np; i++) {
		cprintfd(citoa(rev % 10, numC));
		rev /= 10;
	}
	*magic = '\n';
}

void __attribute__((interrupt)) __attribute__((aligned(4))) 
handler()
{
	uint32_t mepc_value;
	uint32_t mcause_value;
	asm volatile("csrr %0, mepc" : "=r"(mepc_value));
	asm volatile("csrr %0, mcause" : "=r"(mcause_value));
	cprintfi(mepc_value);
	cprintfi(mcause_value);
	for(;;);
}

void
initialise_board(void)
{
	__asm__ volatile("li a0,0" : : : "memory");
	//uint32_t mtvec_value = (uint32_t)handler;
	//uint32_t mstatus_value = 0x8;
	//asm volatile("csrw mstatus, %0" : : "r" (mstatus_value));
	//asm volatile("csrw mtvec, %0" : : "r" (mtvec_value));
	cprintf("Initializing board!\n");
}

void
print_verify_benchmark(int res)
{
	cprintf("Benchmark Return: ");
	cprintfi(res);
}

void __attribute__ ((noinline)) __attribute__ ((externally_visible))
start_trigger(void)
{
	char num1;
	volatile char *magic = 0x20000;
	cprintf("Start trigger!\n");
	start_cycles = get_csr(CYCLE);
	start_instrs = get_csr(INSTR);
}

void __attribute__ ((noinline)) __attribute__ ((externally_visible))
stop_trigger(void)
{
	char num1[50];
	char num2[50];
	uint64_t elps_cycles, elps_instrs;
	elps_cycles = get_csr(CYCLE) - start_cycles;
	elps_instrs = get_csr(INSTR) - start_instrs;
	cprintf("Elapsed Cylces:\n");
	cprintf(citoa(elps_cycles, num1));
	// cprintfi(elps_cycles);
	cprintf("Elapsed Instructions:\n");
	cprintf(citoa(elps_instrs, num2));
	//cprintfi(elps_instrs);
	for(;;);
}

