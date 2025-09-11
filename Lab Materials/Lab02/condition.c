#include <stdio.h>

void main(){
    int arr[5];

    // for (initialization; condition; increment/decrement)
    // for (default i = 0; mandatory condition; default increment i by 1)
    // for (; i < 5;) will work too
    for (int i = 0; i < 5; i++) {
        scanf("%d", &arr[i]);
    }

    int i = 0;
    while (i < 5) {
        if (arr[i] < 100) {
            printf("100 ");
        }
        else if (arr[i] > 1000) {
            printf("999 ");
        }
        else {
            printf("%d ", arr[i]);
        }
        i += 1;
    }
    printf("\n");

    for (int i = 0, j = 1; i < 5; i++, j*= 2) {
        printf("%d %d \n", i, j);
    }
    
    printf("\n");
}