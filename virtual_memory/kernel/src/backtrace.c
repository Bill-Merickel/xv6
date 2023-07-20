#include "stdio.h"
#include "stab.h"
#include "../include/asm/x86.h"
#include "memlayout.h"


void backtrace() {
    cprintf("Backtrace:\n");
    uint ebp = 0;
    read_ebp(ebp);

    while(1) {
        if (ebp == 0 || ebp < KERNLINK || ebp >= DEVSPACE) {
            break;
        }

        uint *rap = (uint *)(ebp + 4);
        uint ra = (uint) *rap;

        struct stab_info info;
        stab_info(ra, &info);

        const char* fname = info.eip_fn_name;
        int fnLength = info.eip_fn_namelen;
        uint fAddress = info.eip_fn_addr;
        uint offset = ra - fAddress;

        cprintf("   <0x%x> ",ra);

        for (int i = 0; i < fnLength; i++) {
            cprintf("%c", fname[i]);
        }

        cprintf("+%d\n", offset);

        uint *next_ebp_pointer = (uint *)ebp;
        uint next_ebp = (uint) *next_ebp_pointer;
        ebp = next_ebp;
    }
}
/*
Backtrace:
   <0xaddress0> [top_of_stack_function_name]+offs1
   <0xaddress1> [next_function_in_stack_name]+offs2
   ...
   <0xaddressN> [last_function_in_stack]+offsN
*/
