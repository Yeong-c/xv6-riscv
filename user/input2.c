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

  int fd = open("README", O_RDONLY);
  char *file = (char *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE, fd, 2275);
  if (file == 0)
    printf("mmap: prot not matched\n\n");

  file = (char *)mmap(0, SIZE, PROT_READ, MAP_POPULATE, fd, 2275);
  printf("freemem: %d\n", freemem());

  for (int i = 0; i < SIZE; i++) {
    char buf[2] = {file[i], '\0'};
    printf("%s", buf);
  }

  file[0] = 'R'; // PROT_READ이므로 write 접근이 막혀서 강제종료되어야
  for (int i = 0; i < SIZE; i++) {
    char buf[2] = {file[i], '\0'};
    printf("%s", buf);
  };

  exit(0);
}

/* 실행 결과
=== TEST START ===
freemem: 32532
mmap: prot not matched

freemem: 32529
run "make qemu".
 */