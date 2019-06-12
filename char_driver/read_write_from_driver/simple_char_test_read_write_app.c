#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h> //sleep
#include <string.h>

#define MEM_SIZE 256
int main()
{
	int fd, count;
	char buf[MEM_SIZE];
	fd = open("/dev/char_device", O_RDWR);
	if(fd == -1)
	{
		printf("failed to open simple char driver\n");
		perror("");
		return EXIT_FAILURE;
	}
	printf("simple driver opened sucesfully fd = %d\n", fd);

	printf("Please enter some msg\n");

	fgets(buf, MEM_SIZE, stdin);

	count = write(fd, buf, strlen(buf));

	printf("%s msg sent to driver of length = %d\n", buf, count);

	memset(buf, 0, sizeof(buf));
	
	count = read(fd, buf, sizeof(buf));
 
	printf("%s msg read from the driver of length = %d\n", buf, count);
	sleep(3);
	close(fd);	
	return EXIT_SUCCESS;
}

