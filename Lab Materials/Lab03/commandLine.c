#include<stdio.h>
#include<stdlib.h>

int main(int argc, char *argv[]) {
    if (argc == 2) {
        printf("The arguments supplied is %s\n", argv[1]);
    }
    else if (argc > 2) {
        printf("Many arguments passed\n");
    }
    else {
        printf("One argument expected\n");
    }
}