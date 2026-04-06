#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    int pid = fork();

    if(pid < 0){
        printf("fork failed\n");
        exit(1);
    }

    if(pid == 0){
        // [자식 프로세스]
        // 우선순위를 10으로 높임 (Weight 증가 -> 더 많은 CPU 시간 할당 기대)
        setnice(getpid(), 10); 
        
        // CPU를 강제로 점유하여 타이머 인터럽트(Tick)와 vruntime을 누적시킴
        // (가상 머신 성능에 따라 루프 숫자를 늘리거나 줄이셔도 됩니다)
        volatile int i;
        for(i = 0; i < 300000000; i++) {
            // Busy waiting...
        }
        
        exit(0);
    } else {
        // [부모 프로세스]
        // 우선순위를 20 (기본값)으로 유지
        setnice(getpid(), 20);
        
        // 자식과 동시에 CPU를 두고 경쟁
        volatile int i;
        for(i = 0; i < 150000000; i++) {
            // Busy waiting...
        }

        // 경쟁 도중(또는 직후)에 시스템 전체 프로세스 상태 출력
        printf("\n=== TEST START ===\n");
        ps(0);

        // 자식 프로세스가 끝날 때까지 대기
        waitpid(pid);
    }

    exit(0);
}