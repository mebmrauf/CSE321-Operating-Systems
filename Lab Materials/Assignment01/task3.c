#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>

void processCount(int *p) {
    __sync_fetch_and_add(p, 1);
}

void callFork(int *count) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }
    if (pid == 0) {
        processCount(count);
        return;
    } else {
        if (pid % 2 != 0) {
            pid_t newChild = fork();
            if (newChild < 0) {
                perror("odd fork failed");
                exit(1);
            }
            if (newChild == 0) {
                processCount(count);
                return;
            }
        }
    }
}

int main(void) {
    int *count = mmap(NULL, sizeof *count, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (count == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    *count = 1;
    pid_t parentPID = getpid();
    callFork(count); //a = fork()
    callFork(count); //b = fork()
    callFork(count); //c = fork()

    while (1) {
        if (wait(NULL) == -1) {
            if (errno == ECHILD) break;
            perror("wait error");
            break;
        }
    }
    if (getpid() == parentPID) {
        printf("Total processes created: %d\n", *count);
    }
    return 0;
}