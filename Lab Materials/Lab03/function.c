#include<stdio.h>
#include<stdlib.h>

int add(int x, int y) {
    int result = x + y;
    return result;
}

int subtract(int m, int n) {
    int result = m - n;
    return result;
}

int swap(int a, int b) {
    int temp;
    temp = a;
    a = b;
    b = temp;

    printf("Values after swap: a = %d, b = %d\n", a, b);
}

int main() {
    int a = 5, b = 10;
    int c = 30, d = 20;
    int sum, difference;

    sum = add(a, b); // function call by value
    printf("The sum of %d and %d is: %d\n", a, b, sum);
    // Note: In C, all function calls are by value, but we can simulate pass by reference using pointers.


    difference = subtract(c, d);
    printf("The difference of %d and %d is: %d\n", c, d, difference);

    printf("Values before swap: a = %d, b = %d\n", a, b);
    swap(a, b);
    
    return 0;
}