#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define DEVICE "/dev/ASSIGN5_DEVICE1"

int main() 
{
	int i,fd;
	char ch, write_buf[100], read_buf[100];
	fd = open(DEVICE, O_RDWR);
	if(fd == -1)
	{
		printf("File %s either does not exist or has been locked by another process\n", DEVICE);
		exit(-1);
	}
	//while(1) {
		printf(" r = read from device, w = write to device, q = quit \n enter command :");
		scanf("%c", &ch);
		//if (ch == 'q') break;

		switch(ch) {
			case 'w':
				printf("Enter Data to write: ");
				scanf(" %[^\n]", write_buf);
				write(fd, write_buf, sizeof(write_buf));
				break;
				
			case 'r':
				read(fd, read_buf, sizeof(read_buf));
				printf("device: %s\n", read_buf);
				break;

			case 'l':
				lseek(fd, 100,SEEK_SET);
				lseek(fd, 100,SEEK_CUR);
				lseek(fd, 100,SEEK_END);
				break;
			default:
				printf("Command not recognized\n");
				close(fd);
				return 0;
				break;
		}
	//}
	close(fd);
	return 0;
}