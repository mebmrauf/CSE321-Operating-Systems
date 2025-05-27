#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
int main(){
printf("1 \n");
pid_t pid = fork();
if(pid == 0) {execl("/bin/ls","ls", NULL);}
else if(pid>0) {execl("/bin/pwd","pwd", NULL);
}}
