#include <stdint.h>

#include <stdint.h>
#include <stdbool.h>

/*
    Enables count-inhibiting for all Zicntr/Zihpm registers.
    Returns the previously inhibited registers.
*/
__attribute__((always_inline)) inline uint32_t __rvb_insight_wrap_begin(void) {
    // TODO abuse jal to find current PC
    uint32_t old_mcountinhibit;

    asm volatile ("csrr %0, mcountinhibit;"
                  "csrs mcountinhibit, 31;" : "=r"(old_mcountinhibit));

    return old_mcountinhibit;
}

/*
    Resets the count-inhibiting state of Zicntr/Zihpm registers to previous value.
*/
__attribute__((always_inline)) inline void __rvb_insight_wrap_end(uint32_t old_mcountinhibit) {
    asm volatile ("csrc mcountinhibit, %0;" : : "r"(~old_mcountinhibit));
}

uint32_t __rvb_insight_read_register(uint32_t reg_num, bool high);

#define WRAP_CALL(...) \
    do { \
        uint32_t var__ = __rvb_insight_wrap_begin(); \
        __VA_ARGS__; \
        __rvb_insight_wrap_end(var__); \
    } while (0)

#define MAX_CFGS 32

enum HPM_VALUES {
    HPM_CYCLE = 0,
    HPM_INSTRET = 2,
    HPM_I_MISS = 3,
    HPM_D_MISS = 4,
    HPM_I_HIT = 5,
    HPM_D_HIT = 6,
    HPM_iTLB_MISS = 7,
    HPM_dTLB_MISS = 8,
    HPM_iTLB_HIT = 9,
    HPM_dTLB_HIT = 10,
    HPM_BB_CYCLES = 11,
    HPM_BR_MISPRE = 12,
    HPM_BR_PRE = 13,
    HPM_FETCH_STALL = 14,
    HPM_EXEC_STALL = 15,
    HPM_MEM_STALL = 16
};

#define HPM_MASK(val) (1u << (val))


#include <stdint.h>
#include <stddef.h>

struct RvbInsightConfig {
    const char *label;
    uint32_t hpm_enable;
};

extern struct RvbInsightConfig cfgs[MAX_CFGS];

void rvb_insight_set_cfg(const char *label, uint32_t hpm_enable);

void rvb_insight_reset_cfgs();

uint32_t rvb_insight_get_cfg_mask(const char *label);

void rvb_insight_set_default_cfg(uint32_t hpm_enable);

size_t rvb_insight_search_cfgs(const char *label);

void rvb_insight_print_default(void);
void rvb_insight_print_mask(uint32_t hpm_enable);
void rvb_insight_print_cfg(const char *label);

#define rvb_insight_print(...) \
    WRAP_CALL( \
        _Generic((0, ##__VA_ARGS__ +0), \
            default: rvb_insight_print_default, \
            uint32_t: rvb_insight_print_mask, \
            char *: rvb_insight_print_cfg \
        ) (__VA_ARGS__) \
    )
