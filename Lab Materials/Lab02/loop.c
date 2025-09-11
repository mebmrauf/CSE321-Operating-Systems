#include <stdio.h>

void main(){
    int arr[5];

    for (int i = 0; i < 5; i++) {
        scanf("%d", &arr[i]);
    }

    int i = 0;
    while (i < 5) {
        printf("%d", arr[i]);
        i += 1;
    }
    printf("\n");
}