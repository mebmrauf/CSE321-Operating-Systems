#include <stdio.h>

int main() {
    int n;
    scanf("%d", &n);

    int arr[n];
    int even = 0, odd = 0;

    // Reading n integers and counting even and odd
    for(int i = 0; i < n; i++) {
        scanf("%d", &arr[i]);
        if(arr[i] % 2 == 0)
            even++;
        else
            odd++;
    }

    if(even == odd) {
        char str[100];
        //scanf("%s", str); // read string without spaces
        scanf("%[^\n]", str); // read string with spaces
        printf("%s %d\n", str, even);
    } else {
        printf("Even numbers: %d\n", even);
        printf("Odd numbers: %d\n", odd);
    }

    return 0;
}