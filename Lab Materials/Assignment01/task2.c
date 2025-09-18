#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t p, q;

    p = fork();
    if (p == 0) {
        q = fork();
        if (q == 0) {
            printf("I am grandchild\n");
        } else if (q > 0) {
            wait(NULL);
            printf("I am child\n");
        } else {
            printf("Failed to fork grandchild");
            exit(1);
        }
    } else if (p > 0) {
        wait(NULL);
        printf("I am parent\n");
    } else {
        printf("Failed to fork child");
        exit(1);
    }

    return 0;
}