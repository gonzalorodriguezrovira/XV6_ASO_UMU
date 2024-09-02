#define LOWEST_PRIO  9
#define NORM_PRIO    5
#define HIGHEST_PRIO 0

#include "types.h"
#include "user.h"

void
do_calc (char* nombre)
{
  int r = 0;

  for (int i = 0; i < 3000; ++i)
  { printf(1, nombre);
    for (int j = 0; j < 1000000; ++j)
      {  
        r += i + j;
      }
  }
  // Imprime el resultado
  printf (1, "\n\n%s: %d prioridad: %d\n\n", nombre, r, getprio(getpid()));
}


int
main(int argc, char *argv[])
{
  if (fork())
    exit(0);

  // El proceso se inicia en baja prioridad.
  // Genera otro proceso hijo que a su vez genera dos
  printf(1, "Hay 4 procesos de minima prioridad ejecutandose, asi que deberian verse intercalados.\n");

  if (fork() == 0)
  {
    if (fork())  // Ambos ejecutan:
    {  setprio(getpid(), LOWEST_PRIO); do_calc("-"); }
    else
    {  setprio(getpid(), LOWEST_PRIO); do_calc("+");}
    
    exit(0);
  }

  if (fork() == 0)
  {
    if (fork())  // Ambos ejecutan:
    {  setprio(getpid(), NORM_PRIO); do_calc("*"); }
    else
    {  setprio(getpid(), LOWEST_PRIO); do_calc("^");}
    
    exit(0);
  }
  
  printf(1, "Me voy a dormir 10 segundos para que puedas interactuar con el shell. \
  Como el shell tiene más prioridad que estos dos procesos, imprimirá sin ser interrumpido por ellos.\n");
  sleep(500);


  printf(1, "Y ahora se lanzan dos de alta prioridad. Deberían mostrarse las prioridades \
  en orden creciente conforme acaban los procesos, y el shell no debería tener interacción.\n");
  printf(1, "Cuando terminen los dos de alta prioridad, deberian seguir los de baja hasta terminar.\n");
  if (fork() == 0)
  {
    if (fork())  // Ambos ejecutan
    {  
      setprio (getpid(), HIGHEST_PRIO); 
      do_calc("0");
      exit(0);
    }
    else
    {  
      setprio (getpid(), HIGHEST_PRIO+1); 
      do_calc("1"); 
      exit(0);}
  }

  exit(0);
}
