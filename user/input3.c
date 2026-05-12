#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#define SIZE 4096

int main()
{
  printf("=== TEST START ===\n");

  printf("freemem: %d\n", freemem());

  int fd = open("README", O_RDWR);
  char *file = (char *)mmap(0, SIZE, PROT_READ | PROT_WRITE, 0, fd, 2275);
  printf("freemem: %d\n", freemem());

  for (int i = 0; i < SIZE; i++) {
    char buf[2] = {file[i], '\0'};
    printf("%s", buf);
  }

  printf("freemem: %d\n\n", freemem());

  file[0] = 'R';

  int pid = fork();
  if (pid < 0) {
    printf("fork failed\n");
    exit(0);
  }
  else if (pid == 0) {
    printf("In child process\n");
    printf("freemem: %d\n", freemem());

    file[15] = '!';
    for (int i = 0; i < SIZE; i++) {
      char buf[2] = {file[i], '\0'};
      printf("%s", buf);
    }
    printf("\n");

    // munmap((uint64)file - MMAPBASE); // 프로세스 종료 시 자동으로 munmap해야
    exit(0);
  }

  waitpid(pid);
  printf("In parent process\n");
  printf("freemem: %d\n", freemem());
  for (int i = 0; i < SIZE; i++) {
    char buf[2] = {file[i], '\0'};
    printf("%s", buf);
  }

  munmap((uint64)file - MMAPBASE);
  printf("freemem: %d\n", freemem());

  exit(0);
}

/* 실행 결과
=== TEST START ===
freemem: 32532
freemem: 32532
run "make qemu".
freemem: 32529

In child process
freemem: 32516
Run "make qemu"!

In parent process
freemem: 32529
Run "make qemu".
freemem: 32530
 */