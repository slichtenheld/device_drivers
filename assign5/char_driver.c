#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>

#define MYDEV_NAME "mycdrv"


struct asp_mycdev {
	char *ramdisk;
	size_t ramdisk_size;
	struct semaphore sem;
	int devNo;
	struct cdev my_cdev;
	// any other field you may want to add
};

static dev_t first;
static unsigned int NUM_DEVICES = 3; // for minor numbers
//static size_t ramdisk_size = (16 * PAGE_SIZE);
static struct class *my_class;
static struct asp_mycdev * mycdev_devices;

module_param(NUM_DEVICES,int,S_IRUGO);

static int my_open(struct inode *inode, struct file *filp) {
	struct asp_mycdev * dev; /* device info */
	pr_info("OPENING device: %s %d %d\n", MYDEV_NAME, imajor(inode), iminor(inode));
	
	dev = container_of(inode->i_cdev, struct asp_mycdev, my_cdev); // coudn't just use iminor?
	filp->private_data = dev; // for other methods
	return 0;
}

static int my_release(struct inode *inode, struct file *filp) {
	pr_info("CLOSING device: %s %d %d\n", MYDEV_NAME, imajor(inode), iminor(inode));
	return 0;
}

static ssize_t my_read(struct file *filp, char __user *buf, size_t lbuf, loff_t *ppos) {
	struct asp_mycdev * dev = filp->private_data;
	if(!down_interruptible(&dev->sem)){
		// down
		int nbytes = 0;
		if ( (lbuf + *ppos) > dev->ramdisk_size) {
			pr_info ("trying to read past end of device");
			return 0;
		}
		nbytes = lbuf - copy_to_user(buf,dev->ramdisk+*ppos,lbuf); // int copy2user(to,from,num bytes 2 copy)
		*ppos += nbytes;
		pr_info("    READING device: %s nbytes=%d, pos=%d\n",MYDEV_NAME,nbytes,(int)*ppos);
		
		// up
		up(&dev->sem);
		return nbytes;
	}
	else {
		pr_info("    READING device: %s FAILED\n",MYDEV_NAME);
		return 0;
	}
}

static ssize_t my_write(struct file *filp, const char __user *buf, size_t lbuf, loff_t *ppos) {
	struct asp_mycdev * dev = filp->private_data;
	if(!down_interruptible(&dev->sem)){
		// down
		int nbytes = 0;
		if ( (lbuf + *ppos) > dev->ramdisk_size) {
			pr_info ("trying to read past end of device");
			return 0;
		}
		nbytes = lbuf - copy_from_user(dev->ramdisk+*ppos,buf,lbuf);
		*ppos += nbytes;
		pr_info("    WRITING device: %s nbytes=%d, pos=%d\n",MYDEV_NAME,nbytes,(int)*ppos);
		
		// up
		up(&dev->sem);
		return nbytes;
	}
	else {
		pr_info("    WRITING device: %s FAILED\n",MYDEV_NAME);
		return 0;
	}
}

static loff_t my_lseek(struct file *filp, loff_t offset, int orig) {
	struct asp_mycdev * dev = filp->private_data;
	if(!down_interruptible(&dev->sem)){
		char * temp;
		// down
		loff_t testpos;
		switch(orig) {
			case SEEK_SET: testpos = offset; break; // beginning of file
			case SEEK_CUR: testpos = filp->f_pos + offset; break; // current pos
			case SEEK_END: testpos = dev->ramdisk_size + offset; break; // EOF
			default: return -EINVAL;
		}
		if (testpos >= dev->ramdisk_size) { // needs to realloc memory 
			while (testpos >= dev->ramdisk_size) { // increase size until pointer is within bounds
				dev->ramdisk_size += PAGE_SIZE;
			}
			temp = krealloc( dev->ramdisk, dev->ramdisk_size, GFP_KERNEL );
			if (temp == NULL) pr_info("    SEEKING device: ERROR reallocating memory");
			else dev->ramdisk = temp;

			pr_info("    SEEKING device: expanding memory");
		}
		else if (testpos < 0 ) testpos = 0;
		filp->f_pos = testpos;
		pr_info("    SEEKING device: %s to pos=%ld", MYDEV_NAME, (long)testpos);

		// up
		up(&dev->sem);
		return testpos;
	}
	else {
		pr_info("    SEEKING device: %s FAILED\n",MYDEV_NAME);
		return 0;
	}
}

static long my_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	return 1;
}

struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.llseek = my_lseek,
	.read = my_read,
	.write = my_write,
	.unlocked_ioctl = my_ioctl,
	.open = my_open,
	.release = my_release,
};

static int __init driver_entry(void) {
	int i;
	
	pr_info("Creating %d devices...\n", NUM_DEVICES);

	if (alloc_chrdev_region(&first, 0, NUM_DEVICES, MYDEV_NAME) < 0) {
		pr_err("mycdrv: failed to allocate character device region\n");
		return -1;
	}
	// if (!(my_cdev = cdev_alloc())) { // unecessary since dcev not a pointer
	// 	pr_err("mycdrv: cdev_alloc() failed\n");
	// 	unregister_chrdev_region(first, N);
	// 	return -1;
	// }

	// init data structures before cdev init and device creation
	mycdev_devices = kmalloc(NUM_DEVICES*sizeof(struct asp_mycdev), GFP_KERNEL); // should check for error

	for (i = 0; i < NUM_DEVICES; i++) {
		mycdev_devices[i].ramdisk_size = PAGE_SIZE;
		mycdev_devices[i].ramdisk = kzalloc(PAGE_SIZE, GFP_KERNEL);
		mycdev_devices[i].devNo = i;
		sema_init(&mycdev_devices[i].sem,1);
		cdev_init(&mycdev_devices[i].my_cdev, &my_fops);
		if (cdev_add(&mycdev_devices[i].my_cdev, MKDEV(MAJOR(first),i), NUM_DEVICES) < 0) {
			pr_err("cdev_add() failed\n");
			cdev_del(&mycdev_devices[i].my_cdev);
			unregister_chrdev_region(first, NUM_DEVICES);
			kfree(mycdev_devices[i].ramdisk);
			return -1;
		}
	}
	//my_custom_struct.ramdisk_size = PAGE_SIZE; 
	//my_custom_struct.ramdisk = kzalloc(my_custom_struct.ramdisk_size, GFP_KERNEL);
	//sema_init(&my_custom_struct.sem,1); //initialize semaphore


	// cdev_init(&(my_custom_struct.my_cdev), &my_fops);
	// if (cdev_add(&my_custom_struct.my_cdev, first, N) < 0) {
	// 	pr_err("cdev_add() failed\n");
	// 	cdev_del(&my_custom_struct.my_cdev);
	// 	unregister_chrdev_region(first, N);
	// 	kfree(my_custom_struct.ramdisk);
	// 	return -1;
	// }

	my_class = class_create(THIS_MODULE, "my_class");

	for (i = 0; i < NUM_DEVICES; i++) {
		device_create(my_class,NULL,MKDEV(MAJOR(first),i),NULL,"%s%d",MYDEV_NAME, i);
	}

	pr_info("Succeeded in registering character device %s\n", MYDEV_NAME);
	
	return 0;
}

static void __exit driver_exit(void) {
	int i;
	for ( i = 0; i < NUM_DEVICES; i++ ) {
		device_destroy(my_class, MKDEV(MAJOR(first),i));
	}
	class_destroy(my_class);
	// delete cdev
	for ( i = 0; i < NUM_DEVICES; i++ ) {
		cdev_del(&mycdev_devices[i].my_cdev);
	}
	//cdev_del(&my_custom_struct.my_cdev);
	// unregister dev_t
	unregister_chrdev_region(first, NUM_DEVICES);
	// free ramdisk
	for ( i = 0; i < NUM_DEVICES; i++ ) {
		kfree(mycdev_devices[i].ramdisk);
	}
	kfree(mycdev_devices);
	//kfree(my_custom_struct.ramdisk); // how to delete each ting?
	// destroy semaphore????
	//sema_destroy(&my_custom_struct.sem);


	pr_info( "mycdrv: unloaded module\n");
}

module_init(driver_entry);
module_exit(driver_exit);

MODULE_LICENSE("GPL v2"); // so that kernel not tainted
