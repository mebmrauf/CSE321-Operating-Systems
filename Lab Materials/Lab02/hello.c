#include <stdio.h>

void main(){
    int a;
    float b = 3.24;
    char c, char2 = 't', char3;
    double d;

    a = 100;
    char3 = 'z';
    
    printf("This is an integer: %d\n", a); // for integer use %d
    printf("This one is %c%c it's a character\n", char2, char3); // for character use %c
    printf("This is a floating number %.2f showing full\n", b); // for float use %f, %.2f for 2 decimal places
    printf("This is a double: %lf\n", d); // for double use %lf
    printf("Let's see all of them %d %c %f \n", a, char3, b);
}