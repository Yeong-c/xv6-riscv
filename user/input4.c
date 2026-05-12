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
  uint addr = mmap(2, SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE, fd, 0);
  if (addr == 0)
    printf("mmap: addr not aligned\n");

  addr = mmap(0, 4095, PROT_READ | PROT_WRITE, MAP_POPULATE, fd, 0);
  if (addr == 0)
    printf("mmap: length not aligned\n");

  int res = munmap(MMAPBASE + 1);
  if (res == -1)
    printf("munmap: not mapped\n\n");
  printf("freemem: %d\n", freemem());


  char *file = (char *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, fd, 0);
  if (file == 0)
    printf("mmap: fd should be -1\n");

  file = (char *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 1);
  if (file == 0)
    printf("mmap: offset should be 0\n\n");


  file = (char *)mmap(0, SIZE, PROT_READ | PROT_WRITE, 0, fd, 2275);
  printf("freemem: %d\n", freemem());

  for (int i = 0; i < SIZE; i++) {
    char buf[2] = {file[i], '\0'};
    printf("%s", buf);
  }
  printf("freemem: %d\n\n", freemem());

  int pid = fork();
  if (pid < 0) {
    printf("fork failed\n");
    exit(0);
  }
  else if (pid == 0) {
    printf("In child process\n");
    printf("freemem: %d\n", freemem());

    file[0] = 'R';

    for (int i = 0; i < SIZE; i++) {
      char buf[2] = {file[i], '\0'};
      printf("%s", buf);
    }

    munmap((uint64)file - MMAPBASE);
    printf("freemem: %d\n\n", freemem());

    file = (char *)mmap(4096, SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE, fd, 2267);
    printf("freemem: %d\n", freemem());

    for (int i = 0; i < SIZE; i++) {
      char buf[2] = {file[i], '\0'};
      printf("%s", buf);
    }
    printf("\n");

    munmap((uint64)file - MMAPBASE);
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
  printf("freemem: %d\n\n", freemem());

  file = (char *)mmap(8192, SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
  printf("freemem: %d\n", freemem());

  file[0] = 'X';
  file[1] = 'V';
  file[2] = '6';
  printf("freemem: %d\n", freemem());

  for (int i = 0; i < SIZE; i++) {
    char buf[2] = {file[i], '\0'};
    printf("%s", buf);
  }
  printf("\n");

  munmap((uint64)file - MMAPBASE);
  printf("freemem: %d\n", freemem());

  close(fd);
  exit(0);
}

/* 실행 결과
=== TEST START ===
freemem: 32532
mmap: addr not aligned
mmap: length not aligned
munmap: not mapped

freemem: 32532
mmap: fd should be -1
mmap: offset should be 0

freemem: 32532
run "make qemu".
freemem: 32529

In child process
freemem: 32516
Run "make qemu".
freemem: 32517

freemem: 32516
you can run "make qemu".

In parent process
freemem: 32529
run "make qemu".
freemem: 32530

freemem: 32530
freemem: 32529
XV6
freemem: 32530
 */