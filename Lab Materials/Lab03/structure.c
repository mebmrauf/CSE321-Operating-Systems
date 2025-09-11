// it's like python's class
#include<stdio.h>
#include<stdlib.h>

struct student {
    char name[50];
    int id;
    float marks;
    float CGPA;

};

int main() {
    struct student s1;
    struct student s2;
    struct student s3;

    s1.id = 22201782;
    s1.marks = 10.5;

    s2.id = 22201949;
    s2.marks = 90.5;

    printf("Student01 ID: %d, Marks: %.2f\n", s1.id, s1.marks);
    printf("Student02 ID: %d, Marks: %.2f\n", s2.id, s2.marks);

    return 0;
}