#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
int main()
{
int fd;
char buffer[80];
static char message[] = "Hello, world";
fd = open("hello.txt",O_RDWR);
if (fd != -1)
{
printf("hello.txt opened for read/write access\n");
write(fd, message, sizeof(message));
lseek(fd, 3, 0);
lseek(fd, 3, SEEK_CUR);/* go back to the beginning of the file */
read(fd, buffer, sizeof(message));
printf(" %s was written to hello.txt \n", buffer);
close (fd);
}
}
