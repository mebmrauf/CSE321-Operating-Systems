#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

void main(){
    pid_t p;
    int status;
    p = fork();
    if (p == 0) {
        printf("Child: I am the child\n"); 
        printf("Child: My PID is %d\n", getpid());
        printf("Child: Parent PID is %d\n", getppid());
    } else if (p>0) {
        printf("Parent: I am the parent\n");
        printf("Parent: Child PID is %d\n", p);
        printf("Parent: My PID is %d\n", getpid());
        wait(&status);
        printf("Parent: Well done child! Proud of you!\n");
    } else {
        printf("Process creation failed\n");
    }
}