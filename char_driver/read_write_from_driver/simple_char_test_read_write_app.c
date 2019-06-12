#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h> //sleep
#include <string.h>

int main()
{
	int fd, count;
	char buf[20];
	fd = open("/dev/char_device", O_RDWR);
	if(fd == -1)
	{
		printf("failed to open simple char driver\n");
		perror("");
		return EXIT_FAILURE;
	}
	printf("simple driver opened sucesfully fd = %d\n", fd);

	printf("Please eneter some msg\n");
	fgets(buf, 20, stdin);
	printf("sending msg to driver  = %s\n", buf);
	count = write(fd, buf, sizeof(buf));
	
	memset(buf, 0, sizeof(buf));
	
	count = read(fd, buf, sizeof(buf));
 
	printf("simple driver read sucesfully  = %s\n", buf);
	sleep(3);
	close(fd);	
	return EXIT_SUCCESS;
}

