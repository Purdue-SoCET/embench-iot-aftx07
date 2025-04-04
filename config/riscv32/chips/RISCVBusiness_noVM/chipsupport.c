#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "chipsupport.h"

void trap_entry();
void pop_tf(trapframe_t*);

extern volatile uint64_t tohost;
extern volatile uint64_t fromhost;

static void do_tohost(uint64_t tohost_value)
{
  tohost = tohost_value;
}

static uint64_t lfsr63(uint64_t x)
{
  uint64_t bit = (x ^ (x >> 1)) & 1;
  return (x >> 1) | (bit << 62);
}

static void cputchar(int x)
{
  do_tohost(0x0101000000000000 | (unsigned char)x);
  asm volatile ("fence.i" : : :);
}

static void cputstring(const char* s)
{
  size_t len = strlen(s);
  for (size_t i = 0; i < len; i++) {
    cputchar(*s++);  
  }
  // cputchar('\n');
}

static void terminate(uint32_t cycles, uint32_t insts)
{
  cputstring("\ncycles: ");
  printhex((uint64_t)cycles);
  cputstring("\ninsts : ");
  printhex((uint64_t)insts);
  cputchar('\n');
  do_tohost(cycles == insts && (cycles == 841 || cycles == 3 || cycles == 0) ? 0 : 1); // if true, then fail otw pass. very hacked, whatevs
  asm volatile ("fence.i" : : :);
  while (1);
}

void wtf()
{
  terminate(841, 841);
}

#define stringify1(x) #x
#define stringify(x) stringify1(x)
#define assert(x) do { \
  if (x) break; \
  cputstring("Assertion failed: " stringify(x) "\n"); \
  terminate(3, 3); \
} while(0)

void printhex(uint64_t x)
{
  char str[17];
  for (int i = 0; i < 16; i++)
  {
    str[15-i] = (x & 0xF) + ((x & 0xF) < 10 ? '0' : 'a'-10);
    x >>= 4;
  }
  str[16] = 0;

  cputstring(str);
}

extern int pf_filter(uintptr_t addr, uintptr_t *pte, int *copy);
extern int trap_filter(trapframe_t *tf);

// void handle_fault(uintptr_t addr, uintptr_t cause)
// {
//   uintptr_t filter_encodings = 0;
//   int copy_page = 1;

//   assert(addr >= DRAM_BASE + PGSIZE && addr < DRAM_BASE + MAX_TEST_PAGES * PGSIZE);
  
//   addr = addr/PGSIZE*PGSIZE;
//   uintptr_t vpn0 = addr - DRAM_BASE;

//   if (user_llpt[vpn0/PGSIZE]) {
//     if (!(user_llpt[vpn0/PGSIZE] & PTE_A)) {
//       user_llpt[vpn0/PGSIZE] |= PTE_A;
//     } else {
//       assert(!(user_llpt[vpn0/PGSIZE] & PTE_D) && cause == CAUSE_STORE_PAGE_FAULT);
//       user_llpt[vpn0/PGSIZE] |= PTE_D;
//     }
//     flush_page(addr);
//     return;
//   }

//   freelist_t* node = freelist_head;
//   assert(node);
//   freelist_head = node->next;
//   if (freelist_head == freelist_tail)
//     freelist_tail = 0;

//   uintptr_t new_pte = (node->addr >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V | PTE_U | PTE_R | PTE_W | PTE_X;

//   if (pf_filter(addr, &filter_encodings, &copy_page)) {
//       new_pte = (node->addr >> PGSHIFT << PTE_PPN_SHIFT) | filter_encodings;
//   }

//   user_llpt[vpn0/PGSIZE] = new_pte | PTE_A | PTE_D;
//   flush_page(addr);

//   asm volatile ("fence.i");

//   assert(user_mapping[vpn0/PGSIZE].addr == 0);
//   user_mapping[vpn0/PGSIZE] = *node;

//   uintptr_t sstatus = set_csr(sstatus, SSTATUS_SUM);
//   memcpy((void*)addr, uva2kva(vpn0), PGSIZE);
//   write_csr(sstatus, sstatus);

//   user_llpt[vpn0/PGSIZE] = new_pte;
//   flush_page(addr);

//   asm volatile ("fence.i");
// }

void handle_trap(trapframe_t* tf)
{
  if (trap_filter(tf)) {
    pop_tf(tf);
  }

  if (tf->cause == CAUSE_USER_ECALL)
  {
    int cycles = tf->gpr[10];
    int insts  = tf->gpr[11];

    terminate(cycles, insts);
  }
  else if (tf->cause == CAUSE_ILLEGAL_INSTRUCTION)
  {
    assert(!"illegal instruction");
    tf->epc += 4;
  }
  else if (tf->cause == CAUSE_FETCH_PAGE_FAULT || tf->cause == CAUSE_LOAD_PAGE_FAULT || tf->cause == CAUSE_STORE_PAGE_FAULT)
    assert(!"unexpected page fault");
  else
    assert(!"unexpected exception");

  pop_tf(tf);
}

static void coherence_torture()
{
  // cause coherence misses without affecting program semantics
  uint64_t random = ENTROPY;
  while (1) {
    uintptr_t paddr = DRAM_BASE + ((random % (2 * (MAX_TEST_PAGES + 1) * PGSIZE)) & -4);
#ifdef __riscv_atomic
    if (random & 1) // perform a no-op write
      asm volatile ("amoadd.w zero, zero, (%0)" :: "r"(paddr));
    else // perform a read
#endif
      asm volatile ("lw zero, (%0)" :: "r"(paddr));
    random = lfsr63(random);
  }
}

void boot(uintptr_t test_addr)
{
  // uint64_t random = ENTROPY;
  // if (read_csr(mhartid) > 0)
  //   coherence_torture();

  _Static_assert(SIZEOF_TRAPFRAME_T == sizeof(trapframe_t), "???");

// #if (MAX_TEST_PAGES > PTES_PER_PT) || (DRAM_BASE % MEGAPAGE_SIZE) != 0
// # error
// #endif
//   // map user to lowermost megapage
//   // l1pt[0] = ((pte_t)user_l2pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
//   l1pt[512] = ((pte_t)user_l2pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
//   // map kernel to uppermost megapage
// #if SATP_MODE_CHOICE == SATP_MODE_SV48
//   l1pt[PTES_PER_PT-1] = ((pte_t)kernel_l2pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
//   kernel_l2pt[PTES_PER_PT-1] = ((pte_t)kernel_l3pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
//   kernel_l3pt[PTES_PER_PT-1] = (DRAM_BASE/RISCV_PGSIZE << PTE_PPN_SHIFT) | PTE_V | PTE_R | PTE_W | PTE_X | PTE_A | PTE_D;
//   user_l2pt[0] = ((pte_t)user_l3pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
//   user_l3pt[0] = ((pte_t)user_llpt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
// #elif SATP_MODE_CHOICE == SATP_MODE_SV39
//   l1pt[PTES_PER_PT-1] = ((pte_t)kernel_l2pt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
//   kernel_l2pt[PTES_PER_PT-1] = (DRAM_BASE/RISCV_PGSIZE << PTE_PPN_SHIFT) | PTE_V | PTE_R | PTE_W | PTE_X | PTE_A | PTE_D;
//   user_l2pt[0] = ((pte_t)user_llpt >> PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
// #elif SATP_MODE_CHOICE == SATP_MODE_SV32
//   l1pt[PTES_PER_PT-1] = (DRAM_BASE/RISCV_PGSIZE << PTE_PPN_SHIFT) | PTE_V | PTE_R | PTE_W | PTE_X | PTE_A | PTE_D;
// #else
// # error
// #endif
//   uintptr_t vm_choice = SATP_MODE_CHOICE;
//   uintptr_t satp_value = ((uintptr_t)l1pt >> PGSHIFT)
//                         | (vm_choice * (SATP_MODE & ~(SATP_MODE<<1)));
//   write_csr(satp, satp_value);
//   if (read_csr(satp) != satp_value)
//     assert(!"unsupported satp mode");

  // Set up PMPs if present, ignoring illegal instruction trap if not.
  uintptr_t pmpc = PMP_NAPOT | PMP_R | PMP_W | PMP_X;
  uintptr_t pmpa = ((uintptr_t)1 << (__riscv_xlen == 32 ? 31 : 53)) - 1;
  asm volatile ("la t0, 1f\n\t"
                "csrrw t0, mtvec, t0\n\t"
                "csrw pmpaddr0, %1\n\t"
                "csrw pmpcfg0, %0\n\t"
                ".align 2\n\t"
                "1: csrw mtvec, t0"
                : : "r" (pmpc), "r" (pmpa) : "t0");

  // set up trap handling
  write_csr(mtvec, trap_entry);
  write_csr(mscratch, read_csr(mscratch));
  write_csr(medeleg,
    // (1 << CAUSE_USER_ECALL) | // want ECALLS to go to M-mode
    (1 << CAUSE_FETCH_PAGE_FAULT) |
    (1 << CAUSE_LOAD_PAGE_FAULT) |
    (1 << CAUSE_STORE_PAGE_FAULT));
  // FPU on; accelerator on; vector unit on
  write_csr(mstatus, MSTATUS_FS | MSTATUS_XS | MSTATUS_VS);
  write_csr(mie, 0);

  // random = 1 + (random % MAX_TEST_PAGES);
  // freelist_head = pa2kva((void*)&freelist_nodes[0]);
  // freelist_tail = pa2kva(&freelist_nodes[MAX_TEST_PAGES-1]);
  // for (long i = 0; i < MAX_TEST_PAGES; i++)
  // {
  //   freelist_nodes[i].addr = DRAM_BASE + (MAX_TEST_PAGES + random)*PGSIZE;
  //   freelist_nodes[i].next = pa2kva(&freelist_nodes[i+1]);
  //   random = LFSR_NEXT(random);
  // }
  // freelist_nodes[MAX_TEST_PAGES-1].next = 0;

  trapframe_t tf;
  memset(&tf, 0, sizeof(tf));
  tf.epc = test_addr;
  tf.gpr[2] = 0x80020000;
  pop_tf(&tf);
}