# Virtual Memory

Here I modified the xv6 virtual memory system, making virtual
memory allocation to be far more efficient by modifying the xv6
memory system to provide much more efficient management of OS memory resources
through the virtual memory system. My primary goal in this lab will be to 
minimize both the amount of wasted memory and the unneeded copying done by 
xv6's current inefficient memory management mechanism.

In particular, I used two optimizations to reduce the amount of
memory used by xv6:

- Copy-on-write forking
- Lazy zero-page allocation.

## Background

The default xv6 virtual memory system naively allocates a physical page per
data-page addressable by user-space processes. This results in poor performance
through two primary metrics: 1) physical pages may be allocated and never used,
wasting memory 2) user-accessible physical pages must be either copied from
other pages (on fork) or zeroed (on allocation), even though they may never be
used. This is highly inefficient, and no modern operating systems eagerly
allocate memory.

Instead, modern OS's rely on lazy allocation, allocating physical space for
virtual memory addresses only when needed. They use two techniques: copy-on-write 
forking and lazy zero initialization.

Copy-on-write forking leverages the fact that on fork both the parent and child
have logically equivalent address spaces. In fact, their address spaces only
diverge when either the parent or the child write to memory. This means the
parent and child can share physical data pages, until either writes to memory.

Lazy zeroing of pages helps reduce the costs of memory allocation, and accessing
zeroed pages. When new memory is allocated by the kernel (e.g. through `sbrk`),
that data contained in that memory must logically be zero. However, just because the
user-space as allocated a page, that doesn't mean they need a true physical
page. If a given data page is never accessed by the process, or only read, the
kernel need not allocate a unique page for it. Instead, the kernel may point
all logically zero-filled user-space pages to a single zero-initialized physical
page, only allocating a unique physical page when the address is written to.

## Copy-on-write forking

When xv6 forks a new process, it clones the parent's entire address space,
creating a newly allocated page in the child for every page in the parent.
This is an expensive and often unneeded operation. Many times the child will
simply call `exec()`, throwing away the address space the parent just carefully
cloned. Additionally, even in the instance the child doesn't fork, it will
rarely touch every page the parent had allocated, and any untouched page is a
wasted copy.

Instead, I delayed page copying on fork
until it is absolutely necessary. The key observation that enables this delay
in work is that at fork both the child and parent will have an identical address
space, and their address spaces will only diverge once either the child or
parent writes memory. I built a copy-on-write implementation for
xv6, in which on fork you will not copy the contents of any user-pages, instead
lazily allocating them as needed (at the time of write). My solution is
transparent to the user-space with the exception that the new implementation will have far more
efficient `fork`s, and more available system memory.

## Zero Initialized Data

When the OS allocates a page for a process (e.g. through sbrk), that page is
zero-initialized (its data is read as zero). So, if a user-process were to
allocate 1000 new pages of virtual address space, each of those 1000 pages would
be allocated as all-zero data. This duplication of user-space data presents
opportunity for optimization within the kernel. A page that is zero-filled need
not be immediately allocated, as the kernel knows that its contents will be all
zero once accessed. Furthermore, if a process only reads the zero data and
never writes it, then the all zero-filled pages can actually be backed by the
same physical page.

I added in zero-initialized data
deduplication and lazy page zero allocation to the kernel. These are the
following design principals I followed:

- Zero-filled virtual addresses should be lazily allocated, allocating a
  unique physical page only on write.
- All zero-initialized virtual pages should share read-only access to a single
  physical zero-page, that is never written.
- Any write to a zero-initialized page will cause a new physical page to be
  allocated and used in its place.
