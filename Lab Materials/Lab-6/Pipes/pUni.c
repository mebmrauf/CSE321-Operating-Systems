#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

int main(){
	int fd[2]; // use this pipe end
	pid_t a;
	char buff[200];
	
	int p=pipe(fd);
	//printf("p: %d\n",p);
	if(p==-1){
		perror("pipe");
	}
	a=fork();
	if(a<0){
		perror("fork");
	}
	else if(a==0){
		// child
		close(fd[0]); // close the read end of the pipe
		printf("Give input:\n");
		read(0,buff,sizeof(buff));
		printf("Writing data for sending_____\n");
		write(fd[1],buff,sizeof(buff));
		printf("Writing done_____\n");
		close(fd[1]); // close the write end of the pipe
	}
	else{
		// parent
		wait(); // wait for the child to complete
		close(fd[1]); // close the write end of the pipe
		printf("Reading data after receiving_____\n");
		read(fd[0],buff,sizeof(buff));
		printf("Data received:\n%s\n",buff);
		close(fd[0]); // close the read end of the pipe
	}

	
	

	return 0;
}
