#include<stdio.h>
#include<stdlib.h>

int main(){
    int x;
    int *ip;
    x = 10;
    ip = &x; // ip now points to x variable

    printf("Address of x: %x\n", &x);
    printf("Address of pointer ip: %x\n", ip);

    printf("Value of x: %d\n", x);
    printf("Value stored in pointed: %d\n", *ip);


    return 0;
}