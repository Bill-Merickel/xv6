# Backtrace and Boot

This branch has added debug functionality to the xv6 kernel and modifies the kernel to support variable memory sizes.

## Backtrace Support

The first modification to the xv6 kernel is the added stack-trace
support through a function named `backtrace`.  Backtrace has the following
specification:

```c
void backtrace();

Summary:
Prints the stack trace of any functions run within the kernel.
```

The format of the data printed is:

```text
Backtrace:
   <0xaddress0> [top_of_stack_function_name]+offs1
   <0xaddress1> [next_function_in_stack_name]+offs2
   ...
   <0xaddressN> [last_function_in_stack]+offsN
```

**NOTE:** there are three space characters (' ') preceding each of the address
lines.

I added a header `backtrace.h` (located in `kernel/include/`) with the
declaration of the `backtrace()` function.

### The Format

The backtrace function will print N+1 lines, where N is the number of functions
in the kernel's stack (excluding the backtrace function itself). The first line
will read `Backtrace:`.

Each of the following N lines will have the following information and format (in
order from left to right):

- Three (3) spaces to start the line
- The address of the next instruction to run in the stack, printed in lower-case
  hex, prefixed by "0x", surrounded on the left by a single `<` and on the right
  by a single `>`
- A single space
- The name of the function on the callstack
- A single `+` character
- A decimal number representing the offset from the start of that function in bytes
- A newline character

So, if my codebase had a function named `foo`, starting on address `0x200` with
length `0x100` (e.g. ending at `0x300`), and my backtrace identified the return
address `0x210` on the stack, I would print the line:

```text
   <0x210> foo+16
```

To create this function, I completed these three tasks:

1. Identified return addresses on the function stack to identify the call-chain
   before this function
2. Looked up the name of the function via its stack location
3. Found the starting address of the function, and calculate its offset

## Modifying Boot

The second modification to the xv6 kernel is modifying the boot system of the kernel
to detect the amount of available RAM on the machine, then passing and utilizing
that information within the xv6 kernel.

### BIOS

#### Boot Loader (bootblock)

Before my modifications, the kernel assumes it has `PHYSTOP` memory (defined in
`include/memlayout.h` as `0xE000000`).  This is a static memory assumption, so
regardless of what the attached machine has, the kernel will use exactly
`PHYSTOP` bytes of RAM. This could be an issue in two ways, (1) the machine has
less than `PHYSTOP` memory, and the kernel assumes it has memory that doesn't
exist! or (2) the machine has much more memory, but the kernel cannot allow the
user-space to utilize it.

To work around this issue, I called into the BIOS and made it tell me
how much memory is available, then I passed this information into our kernel
proper. However, recall that the bios assumes the processor is running in
16-bit real mode (not the more common 32-bit mode that our kernel actually runs
in). This means I have to call the BIOS from the bootblock before it
transitions out of 16-bit real mode and pass the information to the kernel
from there.

There are three primary tasks that I completed to accomplish this:
1. Get the available RAM from the bootblock
2. Make that information available in the kernel
3. Have the kernel read that information, and initialize its free memory
   structure based on the available RAM instead of an arbitrary `PHYSTOP`.
