#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#https://github.com/gonzalorodriguezrovira


int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  int status;
  //Se ontiene el parametro que se le paso a sys_exit
  if(argint(0,&status) < 0)
    return -1;
   //Se le pasa el parametro a exit
  exit(status);
  return 0;  // not reached
}

int
sys_wait(void)
{
  int* status;
  //Se recupera el valor status pasado a sys_wait
  if(argptr(0,(void **)&status,sizeof(*status)) < 0)
    return -1;
  //Se pasa el parametro recuperado a wait()
  return wait(status);
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

  if(argint(0, &n) < 0) //Se recupera el primer argumento
    return -1;
  addr = myproc()->sz; //Se guarda el valor anterior memoria

  if(n<0){ //Se trata si se va a incrementar o a decrementar
    if(growproc(n)<0) //Si se decrementa se libera memoria
      return -1;
  }else
    myproc()->sz += n; //Si se aumenta se aumenta en n el tamaño de la memoria

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
sys_date(void)
{
  acquire(&tickslock);
  struct rtcdate *r;

   // Recoger el parámetro de la primera posición de la pila
  if(argptr(0, (void**)&r, sizeof(*r)) < 0)
    return -1;

  // Llamar a una función para obtener la fecha y hora actual
  cmostime(r);
  release(&tickslock);
  return 0;
}

int
sys_getprio(void){

  int pid;
  if(argint(0, &pid) < 0)
    return -1;

  return getprio(pid);
}

int
sys_setprio(void){

  int pid;
  int aux;
  if(argint(0, &pid) < 0)
    return -1;

  if(argint(1, &aux) < 0)
    return -1;

  unsigned int proc_prio = (unsigned int)aux;

  return setprio(pid,proc_prio);
}

int
sys_phmem(void){
  int pid;
  if(argint(0, &pid) < 0)
    return -1;

  return phmem(pid);
}
