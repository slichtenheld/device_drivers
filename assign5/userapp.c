#include <linux/ioctl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define DEVICE "/dev/mycdrv"

#define CDRV_IOC_MAGIC 'k'
//#define ASP_CLEAR_BUFF _IOWR(CDRV_IOC_MAGIC, 1, int)
#define ASP_CLEAR_BUFF _IO(CDRV_IOC_MAGIC, 1)

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Device number not specified\n");
		return 1;
	}
	int dev_no = atoi(argv[1]);
	char dev_path[20];
	int i,fd;
	char ch[5], write_buf[100], read_buf[10];
	int offset, origin;
	int dir;
	sprintf(dev_path, "%s%d", DEVICE, dev_no);
	fd = open(dev_path, O_RDWR);
	if(fd == -1) {
		printf("File %s either does not exist or has been locked by another "
				"process\n", DEVICE);
		exit(-1);
	}

	printf(" r = read from device after seeking to desired offset\n"
			" w = write to device \n");
	printf(" c = clear the disk\n");
	printf(" o = open file\n");
	printf(" p = close file\n");
	printf(" q = quit\n");
	
	while(ch[0]!='q'){
		printf("\n enter command :");
		fgets(ch, sizeof(ch), stdin);
		switch(ch[0]) {
		case 'w':
			printf("Enter Data to write: ");
			scanf(" %[^\n]", write_buf);
			write(fd, write_buf, sizeof(write_buf));
			printf("Content After write : ");
			lseek(fd, 0, 0);
			if (read(fd, read_buf, sizeof(read_buf)) > 0) {
				printf("%s\n", read_buf);
			} else {
				fprintf(stderr, "Reading failed\n");
			}
			break;

		case 'c':
			printf("Content before clear : ");
			lseek(fd, 0, 0);
			if (read(fd, read_buf, sizeof(read_buf)) > 0) {
				printf("%s\n", read_buf);
			} else {
				fprintf(stderr, "Reading failed\n");
			}
			printf("Sending Clear Command\n");
			int rc = ioctl(fd, ASP_CLEAR_BUFF, 0);
			if (rc == -1)
			{ perror("\n***error in ioctl***\n");
			return -1; }
			printf("Cleared\nNow Reading from begining");
			lseek(fd, 0, 0);
			if (read(fd, read_buf, sizeof(read_buf)) > 0) {
				printf("\ndevice: %s\nend", read_buf);
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
			lseek(fd, offset, origin);
			if (read(fd, read_buf, sizeof(read_buf)) > 0) {
				printf("\ndevice: %s\n", read_buf);
			} else {
				fprintf(stderr, "Reading failed\n");
			}
		break;
		case 'o':
			printf("opening file...\n");
			fd = open(dev_path, O_RDWR);
			if (fd < 0 )
				printf("ERROR opening file\n");
			break;

		case 'p':
			printf("closing file...\n");
			close(fd);
			break;

		case 'q':
			printf("quitting\n");
			break;

		default:
			printf("Command not recognized\n");
			break;
		}
	}
	close(fd);
	return 0;
}
