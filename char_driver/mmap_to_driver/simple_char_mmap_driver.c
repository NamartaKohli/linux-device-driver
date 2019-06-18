#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>  /* printk */
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>  /* cdev struct */
#include <linux/device.h>
#include <linux/slab.h>		/* kmalloc() */
#include <linux/mutex.h>
#include <asm/uaccess.h>	/* copy_*_user */
#include <linux/mm_types.h>
#include <linux/mm.h>


dev_t dev = 0;
static struct class *dev_class;
#define mem_size 4096

int data_copied = 0;

static struct simple_char_dev {
	  
	struct mutex mutex;     /* mutual exclusion semaphore     */
	struct cdev char_cdev;	  /* Char device structure	*/
	char *data;		/*pointer to hold data */
};
struct simple_char_dev *simple_device;

static int simple_char_open(struct inode *inode, struct file *file);
static int simple_char_release(struct inode *inode, struct file *file);
static ssize_t simple_char_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t simple_char_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);
static int simple_char_mmap(struct file *filp, struct vm_area_struct *vma);

static struct file_operations fops =
{
.owner          = THIS_MODULE,
.read           = simple_char_read,
.write          = simple_char_write,
.open           = simple_char_open,
.mmap		= simple_char_mmap,
.release        = simple_char_release,
};

static int simple_char_open(struct inode *inode, struct file *file)
{
	struct simple_char_dev *dev;
	printk(KERN_INFO "Driver Open Function Called...!!!\n");

	dev = container_of(inode->i_cdev, struct simple_char_dev, char_cdev);

	mutex_lock(&dev->mutex);

	dev->data = kzalloc(sizeof(char)*mem_size, GFP_KERNEL);
	if(dev->data == NULL)
	{
		printk(KERN_ERR "failed to allocate memory in write function\n");
		return -ENOMEM;
	}

	memcpy(dev->data,"kernel version", 14);
	file->private_data = dev;

	mutex_unlock(&dev->mutex);
	
        return 0;
}
 
static int simple_char_release(struct inode *inode, struct file *file)
{
	struct simple_char_dev *dev = file->private_data;

	mutex_lock(&dev->mutex);

	if(dev->data)
	{
		kfree(dev->data);
	}
	
	data_copied = 0;
	mutex_unlock(&dev->mutex);
        printk(KERN_INFO "Driver Release Function Called...!!!\n");
        return 0;
}
 
static ssize_t simple_char_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	struct simple_char_dev *dev = filp->private_data;
	
	mutex_lock(&dev->mutex);

	if (copy_to_user(buf, dev->data, strlen(dev->data)))
		return -EFAULT;
	mutex_unlock(&dev->mutex);

        printk(KERN_INFO "Driver Read Function Called...!!!\n");
	
        return strlen(dev->data);
    
}
static ssize_t simple_char_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{

	struct simple_char_dev *dev = filp->private_data;
	
	mutex_lock(&dev->mutex);

	if (copy_from_user(dev->data, buf, len))
		return -EFAULT;

	data_copied = len;
	mutex_unlock(&dev->mutex);

        printk(KERN_INFO "Driver Write Function Called...!!!\n");
	
        return len;
}

static int simple_char_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct simple_char_dev *dev = filp->private_data;

//	printk(KERN_INFO "Driver mmap called msg = %s\n", dev->data); 

	//vma->vm_page_prot = pgprot_cached(vma->vm_page_prot);
	/*remap kernel memory to userspace */
	if ( remap_pfn_range( vma, vma->vm_start, virt_to_phys(dev->data) >> PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot ) ) { 
		printk(KERN_INFO "Simple char driver mmap failed\n"); 
		return -EAGAIN; 
	}
 
	printk(KERN_INFO "Driver mmap called size = %ld\n", vma->vm_end - vma->vm_start); 
	printk(KERN_INFO "Simple char driver mmap OK virt %x \n", vma->vm_start); 
	printk(KERN_INFO " after mapping %s\n", vma->vm_start);
	return 0;
		
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


	simple_device = (struct simple_char_dev *)kmalloc(sizeof(struct simple_char_dev), GFP_KERNEL); 
	if(simple_device == NULL)
	{
		printk(KERN_ERR "failed to allocate memory\n");
		goto err_class;
	}
	
	memset(simple_device, 0, sizeof(struct simple_char_dev));

	mutex_init(&simple_device->mutex);

	/*Creating cdev structure*/
        cdev_init(&simple_device->char_cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&simple_device->char_cdev,dev,1)) < 0){
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
	cdev_del(&simple_device->char_cdev);
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
