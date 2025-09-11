// Pass by reference

#include<stdio.h>
#include<stdlib.h>

void swap (int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;

    return;
}

int main() {
    int a = 5, b = 10;
    printf("Before swap: a = %d, b = %d\n", a, b);
    swap(&a, &b);
    printf("After swap: a = %d, b =%d\n", a, b);
    return 0;
}