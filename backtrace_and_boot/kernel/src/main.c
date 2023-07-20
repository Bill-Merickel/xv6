#include <stdatomic.h>

#include "asm/x86.h"
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "string.h"
#include "stdio.h"

static void startothers(void);
static void mpmain(void)  __attribute__((noreturn));
extern pde_t *kpgdir;
extern char end[]; // first address after kernel loaded from ELF file

// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
int
main(void)
{
  // Store the data of the e820 bios call
  uint* e820_data = (uint*) P2V(0x7e04);

  // Obtain the number of e280 records
  uint num_of_e820_records = *e820_data;

  // Create a pointer to the start of the e820 records
  uint* e820_records = e820_data + 1;

  // Create variables to track the maximum starting physical address
  size_t max_physical_address = 0;

  // Create TRUE_PHYSTOP variable to actually use in kinit calls and initialize it to PHYSTOP (as a minimum value)
  size_t TRUE_PHYSTOP = PHYSTOP;

  // Iterate through the e280 records
  for (int i = 0; i < num_of_e820_records; i++) {

    // Obtain pointers to the starting physical address, length, and type of the current e820 block
    size_t* physical_address_pointer = ((size_t*) e820_records) + i * 6;
    size_t* length_pointer = physical_address_pointer + 2;
    uint* type_pointer = ((uint*) length_pointer) + 2;

    // Dereference these values in separate variables
    size_t physical_address = (size_t) *physical_address_pointer;
    size_t length = (size_t) *length_pointer;
    uint type = (uint) *type_pointer;

    // Check if the type is usable, the physical address is larger than the maximum physical address, and the memory block lies within acceptable regions of the kernel memory space
    if (type == 1 && physical_address > max_physical_address && physical_address + length >= V2P(KERNBASE) && physical_address + length < V2P(DEVSPACE)) {

      // Update the maximum physical address variable
      max_physical_address = physical_address;

      // Update the value of TRUE_PHYSTOP to be equal to the maximum memory block
      TRUE_PHYSTOP = max_physical_address + length;

    // Check if the type is usable and the top of the memory block exceeds the upper bound limit
    } else if (type == 1 && physical_address + length >= V2P(DEVSPACE)) {

      // Set TRUE_PHYSTOP to the highest acceptable region of memory in the kernel memory space before the upper bound limit and break out of the for loop (since the maximunm value was obtained)
      TRUE_PHYSTOP = V2P(DEVSPACE) - 1;
      break;
    }
  }

  kinit1(end, P2V(4*1024*1024)); // phys page allocator
  kvmalloc(TRUE_PHYSTOP); // kernel page table
  mpinit();        // detect other processors
  lapicinit();     // interrupt controller
  seginit();       // segment descriptors
  picinit();       // disable pic
  ioapicinit();    // another interrupt controller
  consoleinit();   // console hardware
  uartinit();      // serial port
  pinit();         // process table
  tvinit();        // trap vectors
  binit();         // buffer cache
  fileinit();      // file table
  ideinit();       // disk 
  startothers();   // start other processors
  kinit2(P2V(4*1024*1024), P2V(TRUE_PHYSTOP), TRUE_PHYSTOP); // must come after startothers()
  userinit();      // first user process
  mpmain();        // finish this processor's setup
}

// Other CPUs jump here from entryother.S.
static void
mpenter(void)
{
  switchkvm();
  seginit();
  lapicinit();
  mpmain();
}

// Common CPU setup code.
static void
mpmain(void)
{
  cprintf("cpu%d: starting %d\n", cpuid(), cpuid());
  idtinit();       // load idt register
  atomic_store(&mycpu()->started, 1); // tell startothers() we're up -- atomically
  scheduler();     // start running processes
}

pde_t entrypgdir[];  // For entry.S

// Start the non-boot (AP) processors.
static void
startothers(void)
{
  extern uchar _binary_entryother_start[], _binary_entryother_size[];
  uchar *code;
  struct cpu *c;
  char *stack;

  // Write entry code to unused memory at 0x7000.
  // The linker has placed the image of entryother.S in
  // _binary_entryother_start.
  code = P2V(0x7000);
  memmove(code, _binary_entryother_start, (uint)_binary_entryother_size);

  for(c = cpus; c < cpus+ncpu; c++){
    if(c == mycpu())  // We've started already.
      continue;

    // Tell entryother.S what stack to use, where to enter, and what
    // pgdir to use. We cannot use kpgdir yet, because the AP processor
    // is running in low  memory, so we use entrypgdir for the APs too.
    stack = kalloc();
    *(void**)(code-4) = stack + KSTACKSIZE;
    *(void(**)(void))(code-8) = mpenter;
    *(int**)(code-12) = (void *) V2P(entrypgdir);

    lapicstartap(c->apicid, V2P(code));

    // wait for cpu to finish mpmain()
    while(atomic_load(&c->started) == 0)
      ;
  }
}

// The boot page table used in entry.S and entryother.S.
// Page directories (and page tables) must start on page boundaries,
// hence the __aligned__ attribute.
// PTE_PS in a page directory entry enables 4Mbyte pages.

__attribute__((__aligned__(PGSIZE)))
pde_t entrypgdir[NPDENTRIES] = {
  // Map VA's [0, 4MB) to PA's [0, 4MB)
  [0] = (0) | PTE_P | PTE_W | PTE_PS,
  // Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
  [KERNBASE>>PDXSHIFT] = (0) | PTE_P | PTE_W | PTE_PS,
};

//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.

