#include "kernel/types.h"
#include "user/user.h"

int main(void)
{
  int p1, p2;

  printf("=== TEST START ===\n");

  p1 = fork();
  if(p1 == 0){
    setnice(getpid(), 0);
    while(1) {
    }
  }

  p2 = fork();
  if(p2 == 0){
    setnice(getpid(), 10);
    while(1) {
    }
  }
  pause(2000);
  ps(0);
  kill(p1);
  kill(p2);
  wait(0);
  wait(0);

  printf("=== TEST END ===\n");
  exit(0);
}