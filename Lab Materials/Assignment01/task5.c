#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t cPID = fork();
    if (cPID < 0) {
        printf("Child fork failed");
        exit(1);
    }

    else if (cPID > 0) {
        printf("1. Parent process ID : 0\n");
        wait(NULL);
    }

    else {
        printf("2. Child process ID: %d\n", getpid());
        for (int i = 0; i < 3; i++) {
            pid_t gcPID = fork();
            if (gcPID < 0) {
                printf("Grandchild fork failed");
                exit(1);
            }
            else if (gcPID > 0) {
                wait(NULL);
            }
            else {
                printf("%d. Grand Child process ID: %d\n", i+3, getpid());
                exit(0);
            }
        }
        exit(0);
    }
    return 0;
}