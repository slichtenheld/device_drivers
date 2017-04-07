#include <linux/ioctl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define DEVICE "/dev/a5"

#define CDRV_IOC_MAGIC 'Z'
#define E2_IOCMODE1 _IOWR(CDRV_IOC_MAGIC, 1, int)
#define E2_IOCMODE2 _IOWR(CDRV_IOC_MAGIC, 2, int)


void open_file(int *fd);
void ioct_1(int fd);
void ioctl_2(int fd);


int main(int argc, char *argv[]) {

	char dev_path[20];
	int i,fd;
	char ch[5];
	char write_buf[100], read_buf[10];
	int offset, origin;
	int dir;
	sprintf(dev_path, "%s", DEVICE);
	
	if(fd == -1) {
		printf("File %s either does not exist or has been locked by another "
				"process\n", DEVICE);
		exit(-1);
	}

	open_file(&fd);
	ioctl_2(fd);
	open_file(&fd);
	ioct_1(fd);
}

void open_file(int* fd){
	printf("opening file...\n");
	char dev_path[20];
	sprintf(dev_path, "%s", DEVICE);
	*fd = open(dev_path, O_RDWR);
	if (*fd < 0 )
		printf("ERROR opening file\n");
}

void ioct_1(int fd){
	if (ioctl(fd, E2_IOCMODE1, 0) < 0 )
		printf("ERROR ioctl 1\n");
}

void ioctl_2(int fd){
	if (ioctl(fd, E2_IOCMODE2, 0) < 0 )
		printf("ERROR ioctl 2\n");
}
