#include "rvb-insight.h"

#ifndef MUTEX_H
#define MUTEX_H

// A simple mutex with a nonatomic lock and blocking lock/unlock methods. Make sure to zero
// initialize!
typedef struct {
    int lock;
} mutex_t;

// Atomically locks the mutex. Will spin until the lock is acquired.
void __attribute__((noinline)) mutex_lock(volatile mutex_t *m);
// Nonatomically unlocks the mutex. Should only be called if the mutex is held.
void __attribute__((noinline)) mutex_unlock(volatile mutex_t *m);

void atomic_set(void *ptr) {
    __asm__ volatile("li t0, 1\n"
                     "1:\n"
                     "lr.w t1, (%[addr])\n"
                     "bnez t1, 1b\n"
                     "sc.w t2, t0, (%[addr])\n"
                     "bnez t2, 1b\n"
                     :
                     : [addr] "r"(ptr)
                     : "t0", "t1", "t2");
}

void atomic_unset(void *ptr) {
    __asm__ volatile("1:\n"
                     "lr.w t0, (%[addr])\n"
                     "sc.w t0, zero, (%[addr])\n"
                     "bnez t0, 1b\n"
                     :
                     : [addr] "r"(ptr)
                     : "t0");
}

void mutex_lock(volatile mutex_t *m) {
    atomic_set((void *)&m->lock);
}

// This should only be called if we hold the lock, so it's okay to nonatomically unset it.
void mutex_unlock(volatile mutex_t *m) {
    m->lock = 0;
}
#endif

#include <string.h>

#define DEFAULT 0

struct RvbInsightConfig cfgs[MAX_CFGS];

void rvb_insight_set_cfg(const char *label, uint32_t hpm_enable)
{
    if (!label) {
        rvb_insight_set_default_cfg(hpm_enable);
        return;
    }
    
    size_t i = rvb_insight_search_cfgs(label);
    if (i == SIZE_MAX) {
        i = rvb_insight_search_cfgs(NULL);
    }
    
    if (i == SIZE_MAX) {
        // TODO print error
    } else {
        cfgs[i] = (struct RvbInsightConfig) {label, hpm_enable};
    }
}

void rvb_insight_reset_cfgs() 
{
    memset(cfgs, 0, sizeof(cfgs));
    rvb_insight_set_default_cfg(INT32_MAX);
}

uint32_t rvb_insight_get_cfg_mask(const char *label)
{
    size_t i = rvb_insight_search_cfgs(label);
    if (i != SIZE_MAX) {
        return cfgs[i].hpm_enable;
    } else {
        // TODO print error
        return -1;
    }
}

void rvb_insight_set_default_cfg(uint32_t hpm_enable)
{
    cfgs[DEFAULT].hpm_enable = hpm_enable;
}

size_t rvb_insight_search_cfgs(const char *label) {
    for (int i = 0; i < MAX_CFGS; i++) {
        if (!label) {
            if (!cfgs[i].label && i != DEFAULT) {
                return i;
            }
        } else {
            if (cfgs[i].label && !strcmp(label, cfgs[i].label)) {
                return i;
            }
        }
    }

    return SIZE_MAX;
}

#include <stdint.h>
#include <stdbool.h>

/*
    Reads a specific register reg_num, either the high or low half.

    Example:
        reg_num == 0 -> cycle
        reg_num == 2 -> instret
*/
uint32_t __rvb_insight_read_register(uint32_t reg_num, bool high) {
    // I don't know if there's a better way to do this...
    // But it (usually) compiles to a single instruction when inlined :)
    uint32_t reg = -1;
    switch (reg_num | ((uint32_t) !!high << 5)) {
        case 0:  asm volatile ("csrr %0, cycle;"        : "=r"(reg)); break;
        case 2:  asm volatile ("csrr %0, instret;"      : "=r"(reg)); break;
        case 3:  asm volatile ("csrr %0, hpmcounter3;"  : "=r"(reg)); break;
        case 4:  asm volatile ("csrr %0, hpmcounter4;"  : "=r"(reg)); break;
        case 5:  asm volatile ("csrr %0, hpmcounter5;"  : "=r"(reg)); break;
        case 6:  asm volatile ("csrr %0, hpmcounter6;"  : "=r"(reg)); break;
        case 7:  asm volatile ("csrr %0, hpmcounter7;"  : "=r"(reg)); break;
        case 8:  asm volatile ("csrr %0, hpmcounter8;"  : "=r"(reg)); break;
        case 9:  asm volatile ("csrr %0, hpmcounter9;"  : "=r"(reg)); break;
        case 10: asm volatile ("csrr %0, hpmcounter10;" : "=r"(reg)); break;
        case 11: asm volatile ("csrr %0, hpmcounter11;" : "=r"(reg)); break;
        case 12: asm volatile ("csrr %0, hpmcounter12;" : "=r"(reg)); break;
        case 13: asm volatile ("csrr %0, hpmcounter13;" : "=r"(reg)); break;
        case 14: asm volatile ("csrr %0, hpmcounter14;" : "=r"(reg)); break;
        case 15: asm volatile ("csrr %0, hpmcounter15;" : "=r"(reg)); break;
        case 16: asm volatile ("csrr %0, hpmcounter16;" : "=r"(reg)); break;
        case 32: asm volatile ("csrr %0, cycleh;"        : "=r"(reg)); break;
        case 34: asm volatile ("csrr %0, instreth;"      : "=r"(reg)); break;
        case 35: asm volatile ("csrr %0, hpmcounter3h;"  : "=r"(reg)); break;
        case 36: asm volatile ("csrr %0, hpmcounter4h;"  : "=r"(reg)); break;
        case 37: asm volatile ("csrr %0, hpmcounter5h;"  : "=r"(reg)); break;
        case 38: asm volatile ("csrr %0, hpmcounter6h;"  : "=r"(reg)); break;
        case 39: asm volatile ("csrr %0, hpmcounter7h;"  : "=r"(reg)); break;
        case 40: asm volatile ("csrr %0, hpmcounter8h;"  : "=r"(reg)); break;
        case 41: asm volatile ("csrr %0, hpmcounter9h;"  : "=r"(reg)); break;
        case 42: asm volatile ("csrr %0, hpmcounter10h;" : "=r"(reg)); break;
        case 43: asm volatile ("csrr %0, hpmcounter11h;" : "=r"(reg)); break;
        case 44: asm volatile ("csrr %0, hpmcounter12h;" : "=r"(reg)); break;
        case 45: asm volatile ("csrr %0, hpmcounter13h;" : "=r"(reg)); break;
        case 46: asm volatile ("csrr %0, hpmcounter14h;" : "=r"(reg)); break;
        case 47: asm volatile ("csrr %0, hpmcounter15h;" : "=r"(reg)); break;
        case 48: asm volatile ("csrr %0, hpmcounter16h;" : "=r"(reg)); break;
    }
    return reg;
}

#include "format.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

static mutex_t rvb_print_mutex = {0};

static void rvb_print_string(const char *str){
    volatile char *MAGIC_ADDR = (volatile char *)0xB0000000;
    for(int i = 0; str[i]; i++){
        *MAGIC_ADDR = str[i];
    }
}

static void print_counters(const char *label, uint32_t mask, uint32_t old_pc){
    char buf[512];
    int idx = 0;

    const char prefix[] = "[rvb-insight] ";
    for(int i = 0; prefix[i]; i++) buf[idx++] = prefix[i];
    for(int i = 0; label[i]; i++) buf[idx++] = label[i];
    char value[24];
    for(int i = 0; i < 32; i++){
        if(!(mask & (1u << i))) continue;
        uint32_t high = __rvb_insight_read_register(i, true);
        uint32_t low = __rvb_insight_read_register(i, false);
        format(" %x %x", value, high, low);
        for(int j = 0; value[j]; j++) buf[idx++] = value[j];
    }
    format(" %x", value, old_pc);
    for(int j = 0; value[j]; j++) buf[idx++] = value[j];
    buf[idx++] = '\n';
    buf[idx++] = '\0';

    mutex_lock(&rvb_print_mutex);
    rvb_print_string(buf);
    mutex_unlock(&rvb_print_mutex);
}

__attribute__((noinline)) void rvb_insight_print_default(void){
    uint32_t old_pc;
    asm volatile ("mv %0, ra" : "=r"(old_pc));
    print_counters(cfgs[0].label, cfgs[0].hpm_enable, old_pc);
}
__attribute__((noinline)) void rvb_insight_print_mask(uint32_t hpm_enable){
    uint32_t old_pc;
    asm volatile ("mv %0, ra" : "=r"(old_pc));
    print_counters("mask", hpm_enable, old_pc);
}
__attribute__((noinline)) void rvb_insight_print_cfg(const char *label){
    uint32_t old_pc;
    asm volatile ("mv %0, ra" : "=r"(old_pc));
    size_t idx = rvb_insight_search_cfgs(label);
    if(idx == SIZE_MAX) return;
    print_counters(cfgs[idx].label, cfgs[idx].hpm_enable, old_pc);
}

//try print bitmask first