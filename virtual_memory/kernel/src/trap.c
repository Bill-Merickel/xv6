#include "asm/x86.h"
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "traps.h"
#include "spinlock.h"
#include "lab2_ag.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;
extern char* global_zero;

// Return the address of the PTE in page table pgdir
// that corresponds to virtual address va.  If alloc!=0,
// create any required page table pages.
static pte_t *
walkpgdir(pde_t *pgdir, const void *va, int alloc)
{
  pde_t *pde;
  pte_t *pgtab;

  pde = &pgdir[PDX(va)];
  if(*pde & PTE_P){
    pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
  } else {
    if(!alloc || (pgtab = (pte_t*)kalloc()) == 0)
      return 0;
    // Make sure all those PTE_P bits are zero.
    // NOTE(lab2): We dont call lab2_pgzero() here, as these are page table
    //   pages, not data pages.
    memset(pgtab, 0, PGSIZE);
    // The permissions here are overly generous, but they can
    // be further restricted by the permissions in the page table
    // entries, if necessary.
    *pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
  }
  return &pgtab[PTX(va)];
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned.

static int
mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm)
{
  //cprintf("trap.c mappages\npgdir: %x\nva: %x\nsize: %x\npa: %x\n\n");
  char *a, *last;
  pte_t *pte;

  a = (char*)PGROUNDDOWN((uint)va);
  last = (char*)PGROUNDDOWN(((uint)va) + size - 1);
  for(;;){
    if((pte = walkpgdir(pgdir, a, 1)) == 0)
      return -1;
    if(*pte & PTE_P)
      panic("remap");
    *pte = pa | perm | PTE_P;
    if(a == last)
      break;
    a += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}


void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//struct trapframe in x86.h
//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  //if trapno is T_SYSCALL, we do syscall()
  if(tf->trapno == T_SYSCALL){
    //get current? process, INT is disabled
    if(myproc()->killed)
      //proc was already killed, exit
      exit();
    //update trapframe for "current?" process
    myproc()->tf = tf;
    //syscall at syscall.c
    //Calls current syscall
    syscall();
    if(myproc()->killed)
      //after we do some things, if proc is killed, exit
      exit();
    return;
  }

  pde_t *d;
  pte_t *pte;
  uint t_pa, flags, cur_v_add;
  char *mem;

  //TEST 11: Guard page?
  switch(tf->trapno){
  case T_PGFLT:
    //cprintf("rcr2(): %x\n",rcr2());
    cur_v_add = PGROUNDDOWN(rcr2());
    d = myproc()->pgdir;
    lab2_report_pagefault(tf);

    if((pte = walkpgdir(d, (void *)cur_v_add, 0)) == 0) {
      kill(myproc()->pid);
      break;
    }

    if ((PTE_FLAGS(*pte) & PTE_P) == 0) {
      kill(myproc()->pid);
      break;
    }

    if ((PTE_FLAGS(*pte) & PTE_G)) {
      //cprintf("Pte_G\n");
      kill(myproc()->pid);
      break;
    }

    if (tf->err >= 4 && ((PTE_FLAGS(*pte) & PTE_U) == 0)) {
      kill(myproc()->pid);
      break;
    }

    if((uint)(P2V(PTE_ADDR(*pte))) == (uint)(global_zero)) {
      if((mem = kalloc()) == 0) {
        kill(myproc()->pid);
        break;
      }
      lab2_pgzero(mem, cur_v_add);
      *pte = *pte & ~PTE_P;
      mappages(d, (char*)cur_v_add, PGSIZE, V2P(mem), PTE_W|PTE_U);
      invlpg((void*) cur_v_add);
      break;
    }

    t_pa = PTE_ADDR(*pte);
    flags = PTE_FLAGS(*pte);

    if ((flags & PTE_W) == 0) {
      if (cow_reference_count[PTE_ADDR(*pte)/PGSIZE] <= 1) {
        *pte = *pte | PTE_W;
      } else {
        cow_reference_count[PTE_ADDR(*pte)/PGSIZE]--;
        if((mem = kalloc()) == 0) {
          kill(myproc()->pid);
          break;
        }
        lab2_pgcopy((mem), (char*)P2V(t_pa), ((cur_v_add)));
        *pte = *pte & ~PTE_P;
        mappages(d, (char*)((cur_v_add)), PGSIZE, V2P(mem), flags|PTE_W|PTE_U);
        invlpg((void*) (cur_v_add));
      }
    } else {
      kill(myproc()->pid);
      break;
    }
    break;
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;

  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  // NOTE(lab2): Disabled preemptive yield for testing.
  /*
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();
  */

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

}