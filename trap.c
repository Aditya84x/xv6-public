#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

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


static void
signalHandlerDefault(struct proc *p, int signum) {
  switch(signum) {      
    case SIGTERM:
      p->killed = 1;
      break;

    case SIGKILL:
      p->killed = 1;
      break;
      
    case SIGVTALRM:
      p->killed = 1;
      break;
    
    case SIGINT:
      p->killed = 1;
      break;
    
    case SIGSTOP:
      p->stopped = 1;
      break;

    case SIGCONT:
      p->stopped = 0;
      break;

    case SIGSEGV:
      p->killed = 1;
      cprintf("Segmentation fault(core dumped): pid %d %s\n", p->pid, p->name);
      break;

    case SIGILL:
      p->killed = 1;
      cprintf("Illegal instruction: pid %d %s\n", p->pid, p->name);
      break;
    
    case SIGUSR1:
      // Default action is to ignore
      break;

    case SIGCHLD:
      // Default action is to ignore
      break;

    default:
      break;
  }
}

void
handle_signals(struct trapframe *tf) {
  if(myproc() && myproc()->killed == 0 && (tf->cs&3) == DPL_USER){
    struct proc *p = myproc();
    // Check for pending signals
    for(int i = 0; i < NSIGNALS; i++){
      if(p->pending_signals[i]){
        cprintf("Process %d handling signal %d\n", p->pid, i);
        if(p->handlers[i] == SIG_DFL){
          signalHandlerDefault(p, i);
        } else if(p->handlers[i] != SIG_IGN){
          p->tf_backup = *tf;
          
          uint esp = tf->esp;
          int arg = i;
          int return_addr = (int)p->return_address; // Address of ulib.c wrapper

          if(return_addr == 0)
            return_addr = 0xFFFFFFFF;

          // Push Argument
          esp -= 4;
          if(copyout(p->pgdir, esp, &arg, 4) < 0) { 
              p->killed = 1; 
              break; 
          }

          // Push Return Address
          esp -= 4;
          if(copyout(p->pgdir, esp, &return_addr, 4) < 0) { 
              p->killed = 1; 
              break; 
          }

          // Jump to Handler
          tf->esp = esp;
          tf->eip = (uint)p->handlers[i];
          
          // Clear pending
          p->pending_signals[i] = 0;
          p->in_signal_handler = 1;
          break;
        }
        p->pending_signals[i] = 0;
      }
    }
  }
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    handle_signals(tf);
    return;
  }

  switch(tf->trapno){
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
  case T_PGFLT:
    cprintf("Page fault at address 0x%x, eip 0x%x, esp 0x%x\n", rcr2(), tf->eip, tf->esp);
    if(myproc() && (tf->cs&3) == DPL_USER){
       // Map Hardware Trap 14 -> Software Signal SIGSEGV
       myproc()->pending_signals[SIGSEGV] = 1;
    } else {
       // Kernel page fault (panic)
       cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
       panic("trap");
    }
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
  
  handle_signals(tf);

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
