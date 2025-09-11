#include <stdio.h>

void main(){
    int a, id;
    float b;
    char c;

    // There is no string type in C, but we can use char arrays
    char name[50];

    //scanf("%d", &a);
    //scanf("%f %c", &b, &c);
    scanf("%s %d", name, &id); // %s can't handle spaces, so it reads until the first space

    printf("Name: %s, ID: %d\n", name, id);
    //printf("From user input %d %f %c \n", a, b, c);
}