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

int
mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm);


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

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{

  int trapexit = tf->trapno+1; //Se recupera y se incrementa el valor de retorno de trap
  
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit(trapexit);//Se devuelve el valor incrementado
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit(trapexit);//Se devuelve el valor incrementado
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
  case T_PGFLT:{ //Se crea el case para manejar el tipo de error
    char *mem; 
    uint va = rcr2(); //Se recupera la direccion virtual que genero el fallo de pagina 
    
    if(va > myproc()->sz || va <= myproc()->userPage){ //Se comprueba que la dirrecion no este por encima de la pila ni que este por debajo de la ultima dirrecion añadida a la pila 
      cprintf("overflow: intento de acceso a pila invalido\n");
      myproc()->killed=1; //Se mata al proceso actual 
      break;
    }
    
    va = PGROUNDDOWN(va); //Se redondea hacia abajo la direccion virtual  
    mem = kalloc(); //Se reserba memoria para una pagina fisica
    
    if(mem == 0){ //Caso fallido de reserva
      cprintf("alloc page out of memory\n");
      kfree(mem); //Se libera la memoria reserbada
      myproc()->killed=1; //Se mata al proceso actual 
      break;
    }
    memset(mem, 0, PGSIZE); //Se reserba memoria del tamaño ed PGSIZE en mem
    if(mappages(myproc()->pgdir, (char*)va, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0){ //Crea una entrada en la tabla de paginas que comienza en la dirección va, de tamaño PGSIZE y lo vincula con la pagina fisica de V2P(men) con esos permisos
      cprintf("alloc page out of memory(2)\n");
      kfree(mem);
      myproc()->killed=1; //Se mata al proceso actual 	
      break;
    }
    break;
    }

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
    exit(trapexit);//Se devuelve el valor incrementado

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit(trapexit);//Se devuelve el valor incrementado
}
