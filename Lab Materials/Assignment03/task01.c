#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>

struct shared {
    char sel[100];
    int b;
};

int main() {
    int sm_id;
    key_t key = 2843;
    int fd[2];
    pid_t opr;

    sm_id = shmget(key, sizeof(struct shared), IPC_CREAT | 0666);
    if (sm_id < 0) {
        perror("shmge");
        exit(1);
    }

    struct shared *shm = (struct shared *)shmat(sm_id, NULL, 0);
    if (shm == (struct shared *)-1) {
        perror("shmat");
        exit(1);
    }

    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    printf("Provide Your Input From Given Options:\n");
    printf("1. Type a to Add Money\n");
    printf("2. Type w to Withdraw Money\n");
    printf("3. Type c to Check Balance\n");

    scanf("%s", (*shm).sel);
    (*shm).b = 1000;

    printf("Your selection: %s\n", (*shm).sel);

    opr = fork();
    if (opr < 0) {
        perror("fork");
        exit(1);
    }

    if (opr == 0) {
        close(fd[0]);

        if (strcmp((*shm).sel, "a") == 0) {
            int addAmount;
            printf("Enter amount to be added:\n");
            scanf("%d", &addAmount);
            if (addAmount > 0) {
                (*shm).b += addAmount;
                printf("Balance added successfully\n");
                printf("Updated balance after addition:\n%d\n", (*shm).b);
            } else {
                printf("Adding failed, Invalid amount\n");
            }
        } else if (strcmp((*shm).sel, "w") == 0) {
            int withdrawAmount;
            printf("Enter amount to be withdrawn:\n");
            scanf("%d", &withdrawAmount);
            if (withdrawAmount > 0 && withdrawAmount <= (*shm).b) {
                (*shm).b -= withdrawAmount;
                printf("Balance withdrawn successfully\n");
                printf("Updated balance after withdrawal:\n%d\n", (*shm).b);
            } else {
                printf("Withdrawal failed, Invalid amount\n");
            }
        } else if (strcmp((*shm).sel, "c") == 0) {
            printf("Your current balance is:\n%d\n", (*shm).b);
        } else {
            printf("Invalid selection\n");
        }

        char msg[] = "Thank you for using\n";
        write(fd[1], msg, strlen(msg));
        close(fd[1]);
        shmdt(shm);
        exit(0);
    } else {
        close(fd[1]);
        char buff[100];
        int n = read(fd[0], buff, sizeof(buff)-1);
        if (n > 0) {
            buff[n] = '\0';
            printf("%s", buff);
        }
        close(fd[0]);
        wait(NULL);
        shmdt(shm);
        shmctl(sm_id, IPC_RMID, NULL);
    }
    return 0;
}