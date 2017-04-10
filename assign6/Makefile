
obj-m := assignment6.o

KERNEL_DIR = /usr/src/linux-headers-$(shell uname -r)

all:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(shell pwd)
	
app: 
	gcc -o dl1 testcase1/dl1.c 
	gcc -o dl2 testcase2/dl2.c
	gcc -o dl3 testcase3/dl3.c -pthread
	gcc -o dl4 testcase4/dl4.c -pthread

clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order *~ dl*
