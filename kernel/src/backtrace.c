#include "backtrace.h"
#include "asm/x86.h"
#include "memlayout.h"
#include "stab.h"
#include "stdio.h"
#include "string.h"

void backtrace()
{
  // Start with initial backtrack function print identifier
  cprintf("Backtrace:\n");

  // Read the value of the %ebp register into the ebp_dest variable
  uint ebp_dest;
  read_ebp(ebp_dest);

  // Iterate while the ebp value lies within acceptable regions of the kernel memory space
  while (ebp_dest >= KERNLINK && ebp_dest < DEVSPACE) {

    // Use pointer arithmetic to obtain the return address of the current frame and print it
    uint return_address = *(uint *)(ebp_dest + 4);
    cprintf("   <0x%x> ", return_address);

    // Generate STAB information
    struct stab_info info;
    stab_info(return_address, &info);

    // Retrieve the function name and name length
    const char* eip_fn_name = info.eip_fn_name;
    int eip_fn_namelen = info.eip_fn_namelen;

    // Print out the function name by iterating through each character
    for (int i = 0; i < eip_fn_namelen; i++) {
      cprintf("%c", eip_fn_name[i]);
    }
    
    // Retrieve the function starting address
    uint eip_fn_starting_address = info.eip_fn_addr;

    // Calculate the function offset and print it
    uint offset = return_address - eip_fn_starting_address;
    cprintf("+%d\n", offset);

    // Update the ebp_dest variable to the base pointer of the next function and continue iteration
    ebp_dest = *(uint *) ebp_dest;
  }
}