#include<stdio.h>
#include<fcntl.h>
int main()
{
int fd;
char buffer[80];
static char message[] = "Hello, world";
fd = open("myfile",O_RDWR);
if (fd != -1)
{
printf("myfile opened for read/write access\n");
write(fd, message, sizeof(message));
lseek(fd, 0, 0); /* go back to the beginning of the file */
read(fd, buffer, sizeof(message));
printf("%s was written to myfile \n", buffer);
close (fd);
}
}
