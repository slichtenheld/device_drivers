#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/semaphore.h>
#include<linux/uaccess.h>
#include<linux/init.h>
#include<linux/slab.h>

#define MYDEV_NAME "ASSIGN5_DEVICE"

struct custom_struct {
	char *ramdisk;
	//size_t ramdisk_size;
	struct semaphore sem;
	int devNo;
	struct cdev my_cdev;
	// any other field you may want to add
} my_custom_struct;

static dev_t first;
static unsigned int count = 1;

static size_t ramdisk_size = (16 * PAGE_SIZE);

static int my_open(struct inode *inode, struct file *file)
{
	pr_info(" OPENING device: %s:\n\n", MYDEV_NAME);
	return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
	pr_info(" CLOSING device: %s:\n\n", MYDEV_NAME);
	return 0;
}

struct file_operations my_fops = {
	.owner = THIS_MODULE,
	//.llseek = my_llseek,
	//.read = my_read,
	//.write = my_write,
	//.ioctl = my_ioctl,
	.open = my_open,
	.release = my_release,
};

static int __init driver_entry(void) {
	if (alloc_chrdev_region(&first, 0, count, MYDEV_NAME) < 0) {
		pr_err("ASSIGN5_DEVICE: failed to allocate character device region\n");
		return -1;
	}
	// if (!(my_cdev = cdev_alloc())) { // unecessary since dcev not a pointer
	// 	pr_err("ASSIGN5_DEVICE: cdev_alloc() failed\n");
	// 	unregister_chrdev_region(first, count);
	// 	return -1;
	// }
	cdev_init(&(my_custom_struct.my_cdev), &my_fops);
	my_custom_struct.ramdisk = kmalloc(ramdisk_size, GFP_KERNEL);
	if (cdev_add(&my_custom_struct.my_cdev, first, count) < 0) {
		pr_err("cdev_add() failed\n");
		cdev_del(&my_custom_struct.my_cdev);
		unregister_chrdev_region(first, count);
		kfree(my_custom_struct.ramdisk);
		return -1;
	}

	pr_info("\nSucceeded in registering character device %s\n", MYDEV_NAME);
	pr_info("Major number = %d, Minor number = %d\n", MAJOR(first),
		MINOR(first));
	return 0;
}

static void __exit driver_exit(void) {
	// delete cdev
	// unregister dev_t
	// free ramdisk

}

module_init(driver_entry);
module_exit(driver_exit);

MODULE_LICENSE("GPL"); // so that kernel not tainted
