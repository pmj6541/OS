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

struct {
    struct spinlock lock;
    struct proc proc[NPROC];
  } ptable;

int
forknexec(const char *path, const char **args)
{
	//fork
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();
  //exec
  char *s, *last;
  int off;
  uint argc, sz, sp, ustack[3+MAXARG+1];
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  pde_t *pgdir, *oldpgdir;

  int nextpid = 1;
  extern void forkret(void);
  extern void trapret(void);
  
  // Allocate process.
  char *sp_alc;
  acquire(&ptable.lock);
  
  for(np = ptable.proc; np < &ptable.proc[NPROC]; np++){
    if(np->state == UNUSED){
      goto found;
    }
  }
    
found:
  np->state = EMBRYO;
  np->pid = nextpid++;
  
  release(&ptable.lock);
  // Allocate kernel stack.
  if((np->kstack = kalloc()) == 0){
    np->state = UNUSED;
    return -2;
  }
  sp_alc = np->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp_alc -= sizeof *np->tf;
  np->tf = (struct trapframe*)sp_alc;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp_alc -= 4;
  *(uint*)sp_alc = (uint)trapret;

  sp_alc -= sizeof *np->context;
  np->context = (struct context*)sp_alc;
  memset(np->context, 0, sizeof *np->context);
  np->context->eip = (uint)forkret;
  
      
  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -2;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;


  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = np->pid;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;
  np->state = RUNNABLE;
  wait(); // wait parent process
  begin_op();

  if((ip = namei((char*)path)) == 0){
    end_op();
    cprintf("exec: fail\n");
    return -1;
  }
  ilock(ip);
  pgdir = 0;


   // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

  if((pgdir = setupkvm()) == 0)
    goto bad;


  // Load program into memory.
  sz = 0;
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
    if(ph.vaddr + ph.memsz < ph.vaddr)
      goto bad;
    if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
      goto bad;
    if(ph.vaddr % PGSIZE != 0)
      goto bad;
    if(loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
      goto bad;
  }

  iunlockput(ip);
  end_op();
  ip = 0;



  // Allocate two pages at the next page boundary.
  // Make the first inaccessible.  Use the second as the user stack.
  sz = PGROUNDUP(sz);
  if((sz = allocuvm(pgdir, sz, sz + 2*PGSIZE)) == 0)
    goto bad;
  clearpteu(pgdir, (char*)(sz - 2*PGSIZE));
  sp = sz;
  

  // Push argument strings, prepare rest of stack in ustack.
  for(argc = 0; args[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp = (sp - (strlen(args[argc]) + 1)) & ~3;
    if(copyout(pgdir, sp, (char *)args[argc], strlen(args[argc]) + 1) < 0)
      goto bad;
    ustack[3+argc] = sp;
  }
  ustack[3+argc] = 0;

  ustack[0] = 0xffffffff;  // fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc+1)*4;  // argv pointer

  sp -= (3+argc+1) * 4;

  if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
    goto bad;

  // Save program name for debugging.
  for(last=s=(char *)path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(curproc->name, last, sizeof(curproc->name));


  // Commit to the user image.
  oldpgdir = curproc->pgdir;
  curproc->pgdir = pgdir;
  curproc->sz = sz;
  curproc->tf->eip = elf.entry;  // main
  curproc->tf->esp = sp;
  switchuvm(curproc);
  freevm(oldpgdir);
  return pid;

bad:
  if(pgdir)
    freevm(pgdir);
  if(ip){
    iunlockput(ip);
    end_op();
  }
  return -1;
}

int
sys_forknexec(void)
{
  char* str;
  char* test;
  //Decode argument using argstr
  argstr(0, &str);
  argstr(1, &test);
  const char **args = (const char**)test;
  const char* path = str;
  return forknexec(path, args);
}

