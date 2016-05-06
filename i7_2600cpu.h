
#ifndef I7_MAJOR
#define I7_MAJOR 0   /* dynamic major by default */
#endif

#ifndef I7_NR_DEVS
#define I7_NR_DEVS 1    /* i70 through i73 */
#endif

/* Use 'k' as magic number */
#define I7_IOC_MAGIC  'k'
#define I7_IOC_MAXNR 14
#define I7_IOCRESET    _IO(I7_IOC_MAGIC, 0)

struct i7_dev {
	struct semaphore sem;     /* mutual exclusion semaphore     */
	struct cdev cdev;	  /* Char device structure		*/
};

//int i7_init_module(void);
//void i7_cleanup_module(void);
//long i7_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
//int i7_release(struct inode *inode, struct file *filp);
//int i7_open(struct inode *inode, struct file *filp);

