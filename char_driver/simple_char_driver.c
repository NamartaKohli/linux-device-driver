#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/device.h>

dev_t dev = 0;
static struct class *dev_class;
static struct cdev char_cdev;

static int simple_char_open(struct inode *inode, struct file *file);
static int simple_char_release(struct inode *inode, struct file *file);
static ssize_t simple_char_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t simple_char_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);

static struct file_operations fops =
{
.owner          = THIS_MODULE,
.read           = simple_char_read,
.write          = simple_char_write,
.open           = simple_char_open,
.release        = simple_char_release,
};

static int simple_char_open(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "Driver Open Function Called...!!!\n");
        return 0;
}
 
static int simple_char_release(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "Driver Release Function Called...!!!\n");
        return 0;
}
 
static ssize_t simple_char_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "Driver Read Function Called...!!!\n");
        return 0;
}
static ssize_t simple_char_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "Driver Write Function Called...!!!\n");
        return len;
}

static int __init simple_char_driver_init(void)
{
	/* allocate major and minor number dynamically */
	if(alloc_chrdev_region(&dev, 0, 1,"char_dev") < 0)
	{
		printk(KERN_ERR "failed to allocate major and minor number\n");
		return -1;
	}
	printk(KERN_ERR "major and minor number are %d %d\n", MAJOR(dev), MINOR(dev));

	/*Creating cdev structure*/
        cdev_init(&char_cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&char_cdev,dev,1)) < 0){
            printk(KERN_INFO "Cannot add the device to the system\n");
            goto err_class;
        }

	/* create struct class */
	/* sys/class/char_class */
	dev_class = class_create(THIS_MODULE, "char_class");
	if(dev_class== NULL)
	{
		printk(KERN_ERR " failed to create the sturuct class for device\n");
		goto err_class;
	}
	/* create device in /dev */
	if((device_create(dev_class, NULL, dev, NULL, "char_device")) == NULL)
	{
		printk(KERN_ERR "failed to create device\n");
		goto err_device;
	}

	return 0;
err_device:
	class_destroy(dev_class);
err_class:
	unregister_chrdev_region(dev,1);
	return -1;
}

static void __exit simple_char_driver_exit(void)
{
	cdev_del(&char_cdev);
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	unregister_chrdev_region(dev,1);
	printk(KERN_ERR "successfully removed the simple char driver\n");
}

module_init(simple_char_driver_init);
module_exit(simple_char_driver_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Namarta Kohli");
MODULE_DESCRIPTION("A simple char driver");
