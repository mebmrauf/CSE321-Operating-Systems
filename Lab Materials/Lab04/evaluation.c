#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

void main(){
    pid_t p;
    pid_t q;
    int status;
    p = fork();
    if (p == 0) {
        printf("Child: I am the child. My PID is %d\n", getpid());
        q = fork();
        if (q == 0) {
        printf("GrandChild: I am the grandchild. My PID is %d\n", getpid());
    }} else if (p>0) {
        printf("Parent: I am the parent. My PID is %d\n", getpid());
        wait(&status);
    } else {
        printf("Process creation failed\n");
    }
}