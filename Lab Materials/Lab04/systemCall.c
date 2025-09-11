#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>


void main(){
    // Read Only
    // int fd = open("testFile", O_RDONLY);
    // char bp[20];
    // read(fd, bp, 10);
    // printf("Read data: %s\n", bp);

    //Write Only
    int fd = open("testFile", O_WRONLY);
    char bp[] = "ulala lala";
    lseek(fd, 0, SEEK_END);
    write(fd, bp, sizeof(bp));
    close(fd);
}