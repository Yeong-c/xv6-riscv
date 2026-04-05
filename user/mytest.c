#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main (int argc, char *argv[]){

int pid = getpid();
int cpid;
int ret;

printf(">>Testing meminfo()\n");
printf("system total memory: %ld bytes\n\n",meminfo());

printf(">>Testing getnice()\n");
printf("Current PID: %d, nice: %d\n",pid, getnice(pid));
printf("Invalid PID (9999) nice: %d\n\n",getnice(9999));

printf(">>Testing setnice()\n");
ret = setnice(pid,10);
printf("Set nice to 10 for PID %d (Return: %d)\n",pid,ret);
printf("Current nice: %d\n\n", getnice(pid));

ret = setnice(pid,-5);
printf("Set invalid nice (-5) for PID %d (Return: %d)\n",pid,ret);
ret = setnice(pid,40);
printf("Set invalid nice (40) for PID %d (Return: %d)\n",pid,ret);
ret = setnice(9999,10);
printf("Set nice for invalid PID 9999 (Return: %d)\n\n",ret);

printf(">>Testing ps()\n");
printf("---- ps(0) : ALL PROCESSES ----\n");
ps(0);
printf("---- ps(%d) : CURRENT PROCESSES ----\n",pid);
ps(pid);
printf("---- ps(9999) : Invalid  PROCESSES(print nothing) ----\n");
ps(9999);
printf("\n");

printf(">>Testing waitpid()\n");
cpid = fork();

if(cpid == 0){	
printf("child is running (PID %d)\n",getpid());
for(volatile int i = 0 ; i < 10000000;i++);
printf("child is exiting (PID %d)\n",getpid());
exit(0);
}

else if (cpid >0){
printf("parent waiting for child (PID %d)\n", cpid);
ret = waitpid(cpid);
printf("waitpid return for child: %d\n", ret);

ret = waitpid(9999);
printf("waitpid return for invalid PID 9999: %d\n", ret);

ret = waitpid(1);
printf("waitpid return for init PID 1: %d\n", ret);
}
else {
printf("fork failed\n");
}
exit(0);
}







