#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  int priority, pid;
  pid = atoi(argv[1]);
  priority = atoi(argv[2]);
  if (priority < 0 || priority > 10){
    printf(2,"Priority should 0-10!\n");
    exit();
  }
  set_proc_priority(pid, priority);
  exit();
}