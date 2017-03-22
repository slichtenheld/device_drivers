#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/semaphore.h>
#include<linux/uaccess.h>
#include<linux/init.h>

MODULE_LICENSE("GPL"); // so that kernel not tainted
#define DEVICE_NAME "ASSIGN5_DEVICE"

struct custom_struct {
	char *ramdisk;
	size_t ramdisk_size;
	struct semaphore sem;
	int devNo;
	struct cdev dev;
	// any other field you may want to add
} my_custom_struct;

static dev_t first;
static unsigned int count = 1;

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
	cdev_init(&(my_custom_struct.my_cdev), my_fops);
	my_custom_struct.ramdisk = kmalloc(ramdisk_size, GFP_KERNEL);
	if (cdev_add(my_cdev, first, count) < 0) {
		pr_err("cdev_add() failed\n");
		cdev_del(my_cdev);
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

// int scull_open(struct inode *inode, struct file *filp)
// {
//     struct scull_dev *dev; /* device information */

//     dev = container_of(inode->i_cdev, struct scull_dev, cdev);
//     filp->private_data = dev;  //for other methods 

//     /* now trim to 0 the length of the device if open was write-only */
//     if ( (filp->f_flags & O_ACCMODE) =  = O_WRONLY) {
//         scull_trim(dev); /* ignore errors */
//     }
//     return 0;          /* success */
// }
// module_init(driver_entry);
// module_exit(driver_exit);

// struct fake_device 
// {
// 	char data[100];
// 	struct semaphore sem;
// } virtual_device;

// struct cdev *mcdev;
// int major_number;
// int ret;
// dev_t dev_num;
// #define DEVICE_NAME "newTestDevice"

// int device_open(struct inode *inode, struct file *filp)
// {
// 	if(down_interruptible(&virtual_device.sem)!=0)
// 	{
// 		printk(KERN_ALERT "newTestDevice: could not lock device during open");
// 		return -1;
// 	}
// 	printk(KERN_INFO "newTestDevice: Opened Device");
// 	return 0;
// }

// ssize_t device_read(struct file *filp, char *bufStoreData, size_t bufCount, loff_t *curOffset)
// {
// 	printk(KERN_INFO "newTestDevice: Reading from device");
// 	ret = copy_to_user(bufStoreData, virtual_device.data, bufCount);
// 	return ret;
// }

// ssize_t device_write(struct file *filp, const char *bufSourceData, size_t bufCount, loff_t *curOffset)
// {
// 	printk(KERN_INFO "newTestDevice: Writing to device");
// 	ret = copy_from_user(virtual_device.data, bufSourceData, bufCount);
// 	return ret;
// }

// int device_close(struct inode *inode, struct file *flip)
// {
// 	up(&virtual_device.sem);
// 	printk(KERN_INFO "newTestDevice: Closed Device");
// 	return 0;
// }

// struct file_operations fops = {
// 	.owner = THIS_MODULE,
// 	.open = device_open,
// 	.release = device_close,
// 	.write = device_write,
// 	.read = device_read
// };

// // variable number of devices at load time, default will be three
// static int __init driver_entry(void)
// {
// 	ret = alloc_chrdev_region(&dev_num,0,1,DEVICE_NAME); //dynamically assigns major
// 	if (ret<0) {
// 		printk(KERN_ALERT "newTestDevice: failed to allocate major number");
// 		return ret;
// 	}
// 	major_number = MAJOR(dev_num);
// 	printk(KERN_INFO "newTestDevice: major number : %d", major_number);
// 	printk(KERN_INFO "newTestDevice: use \"mknod /dev/%s c %d\" for device file.", DEVICE_NAME, major_number);
// 	mcdev = cdev_alloc();
// 	mcdev->ops = &fops;
// 	mcdev->owner = THIS_MODULE;
// 	ret = cdev_add(mcdev, dev_num, 1);
// 	if(ret<0)
// 	{
// 		printk(KERN_ALERT "newTestDevice: failed to add cdev to kernel");
// 		return ret;
// 	}
// 	sema_init(&virtual_device.sem,1);
// 	return 0;
// }

// static void __exit driver_exit(void)
// {
// 	cdev_del(mcdev);
// 	unregister_chrdev_region(dev_num,1);
// 	printk(KERN_ALERT "newTestDevice: unloaded module");
// }


// module_init(driver_entry);
// module_exit(driver_exit);
