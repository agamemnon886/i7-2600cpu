#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>

#include <asm/uaccess.h>	/* copy_*_user */
#include "i7_2600cpu.h"	/* copy_*_user */

MODULE_AUTHOR("Kirill B");
MODULE_LICENSE("Dual BSD/GPL");

int i7_major =   I7_MAJOR;
int i7_minor =   0;
int i7_nr_devs = I7_NR_DEVS;	/* number of bare i7 devices */

struct i7_dev *i7_devices;	/* allocated in i7_init_module */

/*
 * Open and close
 */

static int i7_open(struct inode *inode, struct file *filp)
{
	struct i7_dev *dev; /* device information */

	printk(KERN_NOTICE "%s start\n", __FUNCTION__);

	dev = container_of(inode->i_cdev, struct i7_dev, cdev);
	filp->private_data = dev; /* for other methods */

	/* now trim to 0 the length of the device if open was write-only */
	if ( (filp->f_flags & O_ACCMODE) == O_WRONLY) {
		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
		/* TODO: */
		up(&dev->sem);
	}
	return 0;          /* success */
}

static int i7_release(struct inode *inode, struct file *filp)
{
	printk(KERN_NOTICE "%s start\n", __FUNCTION__);
	return 0;
}

/*
 * The ioctl() implementation
 */

static long i7_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	printk(KERN_NOTICE "%s start\n", __FUNCTION__);
	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if (_IOC_TYPE(cmd) != I7_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > I7_IOC_MAXNR) return -ENOTTY;

	switch(cmd) {
	  case I7_IOCRESET:
		break;
	  default:  /* redundant, as cmd was checked against MAXNR */
		return -ENOTTY;
	}
	return ret;

}

struct file_operations i7_fops = {
	.owner =    THIS_MODULE,
	.unlocked_ioctl = i7_ioctl,
	.open =     i7_open,
	.release =  i7_release,
};

/*
 * The cleanup function is used to handle initialization failures as well.
 * Thefore, it must be careful to work correctly even if some of the items
 * have not been initialized
 */
static void i7_cleanup_module(void)
{
	dev_t devno = MKDEV(i7_major, i7_minor);

	printk(KERN_INFO "%s start\n", __FUNCTION__);
	/* Get rid of our char dev entries */
	if (i7_devices) {
		cdev_del(&i7_devices->cdev);
		kfree(i7_devices);
	}

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, i7_nr_devs);
	printk(KERN_INFO "%s end\n", __FUNCTION__);
}

static int i7_init_module(void)
{
	int ret;
	dev_t dev = 0;

	printk(KERN_INFO "%s start\n", __FUNCTION__);
/*
 * Get a range of minor numbers to work with, asking for a dynamic
 * major unless directed otherwise at load time.
 */
	ret = alloc_chrdev_region(&dev, i7_minor, i7_nr_devs, "i7_2600cpu");
	i7_major = MAJOR(dev);
	if (ret < 0) {
		printk(KERN_WARNING "i7: can't get major %d\n", i7_major);
		return ret;
	}

	/*
	 * allocate the devices -- we can't have them static, as the number
	 * can be specified at load time
	 */
	i7_devices = kmalloc(sizeof(struct i7_dev), GFP_KERNEL);
	if (!i7_devices) {
		printk(KERN_WARNING "i7: can't allocate memory for device structure \n");
		ret = -ENOMEM;
		goto fail;  /* Make this more graceful */
	}
	memset(i7_devices, 0, sizeof(struct i7_dev));

	/* Initialize device structure */
	sema_init(&i7_devices->sem, 1);
	cdev_init(&i7_devices->cdev, &i7_fops);
	i7_devices->cdev.owner = THIS_MODULE;
	ret = cdev_add(&i7_devices->cdev, dev, 1);
	if (ret) {
		printk(KERN_NOTICE "Error %d", ret);
		goto fail;
	}

	printk(KERN_INFO "%s end\n", __FUNCTION__);
	return 0; /* succeed */

fail:
	i7_cleanup_module();
	return ret;
}

module_init(i7_init_module);
module_exit(i7_cleanup_module);

