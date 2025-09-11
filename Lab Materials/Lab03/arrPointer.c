#include<stdio.h>
#include<stdlib.h>

int main() {
    int arr[5] = {10, 20, 30, 40, 50};
    int *ip;
    ip = arr; // no need to use & operator for arrays

    for (int i = 0; i < 5; i++) {
        printf("Address of arr[%d]: %x\n", i, ip);
        printf("Value stored in arr[%d]: %d\n", i, *ip);
        ip++; // move to the next integer in the array
    }

    return 0;
}