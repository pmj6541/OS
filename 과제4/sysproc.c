#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"
#include "spinlock.h"

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

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
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

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

void sys_set_proc_priority(int pid, int priority){
  struct proc *p;
  int pid_sys;
  int priority_sys;
  argint(0,&pid_sys);
  argint(1,&priority_sys);
  acquire(&ptable.lock); 

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid_sys){
      p->priority = priority_sys;
      break;
    }
  }
  release(&ptable.lock);
  return;
}

int sys_get_proc_priority(int pid){
  struct proc *p;
  int pid_sys;
  argint(0,&pid_sys);

  acquire(&ptable.lock); 
  int priority = -1;
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid_sys){
      priority = p->priority;
      break;
    }
  }
  release(&ptable.lock);
  return priority;
}

int
sys_state(void){
  return state();
}

uint 
sys_getNumFreePages(void)
{
  return getNumFreePages();
}


