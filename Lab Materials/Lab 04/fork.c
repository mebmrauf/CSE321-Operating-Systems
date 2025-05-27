#include <sys/types.h>
main()
{
pid_t pid;
pid = fork();
if (pid == 0)
printf("\n I'm the child process\n");
else if (pid > 0)
printf("\n I'm the parent process. My child pid is %d \n", pid);
else
perror("error in fork");
}
