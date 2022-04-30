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

static inline void
cprintf(char * str) {
	size_t loop_idx;
	volatile char *magic = 0x20000;
	for (loop_idx = 0; str[loop_idx] != '\0'; loop_idx++) {
		*magic = str[loop_idx];
	}
	*magic = '\n';
}

void
initialise_board(void)
{
	__asm__ volatile("li a0,0" : : : "memory");
	cprintf("Initializing board!\n");
}

void __attribute__ ((noinline)) __attribute__ ((externally_visible))
start_trigger(void)
{
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
	cprintf("Elapsed Instructions:\n");
	cprintf(citoa(elps_instrs, num2));
	for(;;);
}

