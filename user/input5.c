#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#define SIZE 8192

int main()
{
  printf("=== TEST START ===\n");

  printf("freemem: %d\n", freemem());

  int fd = open("README", O_RDWR); // README 내용을 복사하여 붙여넣기 진행 (파일 길이를 늘리기 위해)
  char *file = (char *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE, fd, 0);
  printf("freemem: %d\n", freemem());

  for (int i = 0; i < SIZE; i++) {
    char buf[2] = {file[i], '\0'};
    printf("%s", buf);
  }
  printf("\n\n");

  munmap((uint64)file - MMAPBASE);
  printf("freemem: %d\n", freemem());

  close(fd);
  exit(0);
}

/* 실행 결과
=== TEST START ===
freemem: 32532
freemem: 32528
... README 내용 출력 ...
freemem: 32530
 */