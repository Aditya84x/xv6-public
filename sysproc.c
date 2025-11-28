#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;
  int signum;

  if(argint(0, &pid) < 0)
    return -1;

  if(argint(1, &signum) < 0)
    return -1;
  
  if(signum < 0 || signum >= NSIGNALS)
    return -1;
  
  return kill(pid, signum);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_setsighandler(void) {
  int signum;
  sighandler_t handler;
  sighandler_t old_handler;
  sighandler_t sigreturn_addr;
  struct proc *currproc = myproc();
  if(argint(0, &signum) < 0) { // this gives value of signum
    return -1;
  }
  if(argint(1, (int*)&handler) < 0) { // this gives value of handler function pointer
    return -1;
  }
  if(argint(2, (int*)&sigreturn_addr) < 0) { // this gives value of sigreturn function pointer
    return -1;
  }

  currproc->return_address = sigreturn_addr;

  if(signum < 0 || signum >= NSIGNALS || signum == SIGKILL || signum == SIGSTOP) {
    return -1;
  }

  old_handler = currproc->handlers[signum];
  currproc->handlers[signum] = handler;
  return (int)old_handler;
}

int
sys_sigreturn(void) {
  struct proc *curproc = myproc();
  if(curproc == 0) {
    return -1;
  }
  // Restore the original trapframe
  *curproc->tf = curproc->tf_backup;
  curproc->in_signal_handler = 0;
  return 0;
}

int
sys_waitpid(void)
{
  int pid;
  int *status;
  
  if(argint(0, &pid) < 0)
    return -1;
  if(argptr(1, (void*)&status, sizeof(int)) < 0)
    status = 0;

  return waitpid(pid, status);
}
