#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h> //sleep
#include <string.h>
#include <sys/mman.h>

#define MEM_SIZE 256
int main()
{
	int fd, count, ret;
	char buf[MEM_SIZE];
	fd = open("/dev/char_device", O_RDWR);
	if(fd == -1)
	{
		printf("failed to open simple char driver\n");
		perror("");
		return EXIT_FAILURE;
	}
	printf("simple driver opened sucesfully fd = %d\n", fd);

	char *addr = (char *)mmap(NULL, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if(addr == MAP_FAILED)
	{
		printf("failed to mmap\n");
		close(fd);
		return EXIT_FAILURE;
	}
	
	printf("mmap success with address = %x\n", addr);
	sleep(3);
	printf("mmap success with address = %s\n", addr);

	count = read(fd, buf, sizeof(buf));

        printf("%s msg read from the driver of length = %d\n", buf, count);

	munmap(addr, MEM_SIZE);
	close(fd);	
	return EXIT_SUCCESS;
}

