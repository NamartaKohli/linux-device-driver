#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h> //sleep

int main()
{
	int fd;
	fd = open("/dev/char_device", O_RDWR);
	if(fd == -1)
	{
		printf("failed to open simple char driver\n");
		perror("");
		return EXIT_FAILURE;
	}
	printf("simple driver opened sucesfully fd = %d\n", fd);
	sleep(3);
	close(fd);	
	return EXIT_SUCCESS;
}

