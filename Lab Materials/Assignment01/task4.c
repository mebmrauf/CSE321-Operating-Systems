#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void merge(int arr[], int l, int m, int r) {
    int i = l, j = m + 1, k = 0;
    int temp[r - l + 1];

    while (i <= m && j <= r) {
        if (arr[i] >= arr[j])
            temp[k++] = arr[i++];
        else
            temp[k++] = arr[j++];
    }

    while (i <= m)
        temp[k++] = arr[i++];
    while (j <= r)
        temp[k++] = arr[j++];

    for (i = l, k = 0; i <= r; i++, k++)
        arr[i] = temp[k];
}

void mergeSort(int arr[], int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

int main(int argc, char *argv[]) {
    if (argc >= 2) {
        int n = argc - 1;
        int arr[n];

        for (int i = 0; i < n; i++) {
            arr[i] = atoi(argv[i + 1]);
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            return 1;
        } else if (pid == 0) {
            mergeSort(arr, 0, n - 1);
            printf("Sorted Array(child):\n");
            for (int i = 0; i < n; i++)
                printf("%d ", arr[i]);
            printf("\n");
            exit(0);
        } else {
            wait(NULL);
            printf("Odd/Even(parent):\n");
            for (int i = 0; i < n; i++) {
                char *parity;

                if (arr[i] % 2 == 0) {
                    parity = "Even";
                } else {
                    parity = "Odd";
                }

                printf("%d is %s\n", arr[i], parity);
            }
        }
    } else {
        printf("One argument expected\n");
    }

    return 0;
}