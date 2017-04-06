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

int main(int argc, char *argv[]) {
	// if (argc < 1) {
	// 	fprintf(stderr, "Device number not specified\n");
	// 	return 1;
	// }
	//int dev_no = atoi(argv[1]);
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

	printf(" r = read from device after seeking to desired offset\n"
			" w = write to device \n");
	printf(" c = close file\n");
	printf(" o = open file\n");

	
	
	while (ch[0]!='q'){
		printf("\nenter command :");
		//scanf("%c", &ch);
		fgets(ch,sizeof(ch),stdin);
		switch(ch[0]) {
			case 'w':
				printf("Enter Data to write: ");
				scanf(" %[^\n]", write_buf);
				write(fd, write_buf, sizeof(write_buf));
				printf("Content After write : ");
				
				if (read(fd, read_buf, sizeof(read_buf)) > 0) {
					printf("%s\n", read_buf);
				} else {
					fprintf(stderr, "Reading failed\n");
				}
				break;

			case 'r':
				printf("Origin \n 0 = beginning \n 1 = current \n\n");
				printf(" enter origin :");
				scanf("%d", &origin);
				printf(" \n enter offset :");
				scanf("%d", &offset);
				
				if (read(fd, read_buf, sizeof(read_buf)) > 0) {
					printf("\ndevice: %s\n", read_buf);
				} else {
					fprintf(stderr, "Reading failed\n");
				}
				break;

			case '1':
				if (ioctl(fd, E2_IOCMODE1, 0) < 0 )
					printf("ERROR ioctl 1\n");
				break;

			case '2':
				if (ioctl(fd, E2_IOCMODE2, 0) < 0 )
					printf("ERROR ioctl 2\n");
				break;


			case 'o':
				printf("opening file...\n");
				fd = open(dev_path, O_RDWR);
				if (fd < 0 )
					printf("ERROR opening file\n");
				break;

			case 'c':
				printf("closing file...\n");
				close(fd);
				break;


			case 'q':
				printf("quitting...\n");
				break;

			default:
				printf("%s\n", ch);
		}
	}
	//close(fd);
	return 0;
}
