#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char *parity;

    if (argc >= 2) {
        for (int i = 1; i < argc; i++) {
            int num = atoi(argv[i]);

            if (num % 2 == 0) {
                parity = "Even";
            } else {
                parity = "Odd";
            }

            printf("%d is %s\n", num, parity);
        }
    } else {
        printf("One argument expected\n");
    }

    return 0;
}