#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{ 
  double x = 0.00;
  for(float j=0.1;j<100000.0;j+=0.001){
    x += 11.11 * 11.11;
  }

  exit(); 
}
