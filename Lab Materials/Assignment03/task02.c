#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

struct msg {
    long int type;
    char txt[6];
};

#define loginOTP 1
#define OTPtoLogin 2
#define OTPtoMail 3
#define mailtoLogin 4

int main() {
    int messageID;
    key_t key;
    struct msg message;
    char workspace[7];
    pid_t pidOTP, pidMAIL;
    char OTPgen[6];
    char OTPmail[6];

    key = 1234;
    messageID = msgget(key, IPC_CREAT | 0666);

    if (messageID == -1) {
        perror("msgget failed");
        exit(1);
    }
    printf("Please enter the workspace name:\n");
    scanf("%6s", workspace);
    workspace[6] = '\0';

    if (strcmp(workspace, "cse321") != 0) {
        printf("Invalid workspace name\n");
        msgctl(messageID, IPC_RMID, NULL);
        exit(0);
    }

    message.type = loginOTP;
    strcpy(message.txt, workspace);
    msgsnd(messageID, &message, sizeof(message.txt), 0);
    printf("Workspace name sent to otp generator from log in: %s\n", workspace);

    pidOTP = fork();

    if (pidOTP == 0) {
        struct msg msg_recv, messageSend;
        msgrcv(messageID, &msg_recv, sizeof(msg_recv.txt), loginOTP, 0);
        printf("OTP generator received workspace name from log in: %s\n", msg_recv.txt);

        pid_t myPID = getpid();
        int OTPval = (myPID % 90000) + 10000;
        for (int i = 4; i >= 0; i--) {
            messageSend.txt[i] = (OTPval % 10) + '0';
            OTPval /= 10;
        }
        messageSend.txt[5] = '\0';

        messageSend.type = OTPtoLogin;
        msgsnd(messageID, &messageSend, sizeof(messageSend.txt), 0);
        printf("OTP sent to log in from OTP generator: %s\n", messageSend.txt);

        messageSend.type = OTPtoMail;
        msgsnd(messageID, &messageSend, sizeof(messageSend.txt), 0);
        printf("OTP sent to mail from OTP generator: %s\n", messageSend.txt);

        pidMAIL = fork();

        if (pidMAIL == 0) {
            struct msg mailReceive, mailSend;
            msgrcv(messageID, &mailReceive, sizeof(mailReceive.txt), OTPtoMail, 0);
            printf("Mail received OTP from OTP generator: %s\n", mailReceive.txt);

            strcpy(mailSend.txt, mailReceive.txt);
            mailSend.type = mailtoLogin;
            msgsnd(messageID, &mailSend, sizeof(mailSend.txt), 0);
            printf("OTP sent to log in from mail: %s\n", mailSend.txt);
            exit(0);
        } else {
            wait(NULL);
            exit(0);
        }
    } else {
        wait(NULL);
        msgrcv(messageID, &message, sizeof(message.txt), OTPtoLogin, 0);
        printf("Log in received OTP from OTP generator: %s\n", message.txt);
        strcpy(OTPgen, message.txt);

        msgrcv(messageID, &message, sizeof(message.txt), mailtoLogin, 0);
        printf("Log in received OTP from mail: %s\n", message.txt);
        strcpy(OTPmail, message.txt);

        if (strcmp(OTPgen, OTPmail) == 0) {
            printf("OTP Verified\n");
        } else {
            printf("OTP Incorrect\n");
        }
        msgctl(messageID, IPC_RMID, NULL);
    }

    return 0;
}