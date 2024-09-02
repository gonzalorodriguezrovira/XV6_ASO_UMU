#include "types.h"
#include "stat.h"
#include "user.h"


char array[4096];

int
main(int argc, char *argv[])
{
  int pid, physical_mem;

  char *memory = sbrk(32768*sizeof(char));


  memory[0] = 'A';
  memory[8192] = 'B';

  array[0] = 'A';

  pid = getpid();
  
  physical_mem = phmem(pid);

  if (physical_mem == -1)
  {
    printf(2, "phmem: PID %d does not exist\n", pid);
    exit(0);
  }
 
  printf(1, "Physical memory occupied by PID %d: %d KiBytes\n", pid, physical_mem);
  printf(1, "Output: 24 KiB\n");

  exit(0);
}
