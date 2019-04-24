#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/device.h>
#include<linux/interrupt.h>
#include <linux/platform_device.h>

/*you can use any interrupt line which is shared in your system
  you can get this information from cat /proc/interrupts */

static void my_tasklet_handler(unsigned long flag);

unsigned int irq = 15; 

DECLARE_TASKLET(my_tasklet, my_tasklet_handler, 156);


static void my_tasklet_handler(unsigned long flag)
{
	
	printk(KERN_ALERT " inside %s with data %lu\n", __func__, flag);
	return;
}


static irqreturn_t simple_interrupt_handler (int irq, void *dev)
{
	static int count =0;
	if(count==0) {
		printk(KERN_ALERT " Interrupt occured on IRQ %d\n", irq);
		tasklet_schedule(&my_tasklet);
		count++;
	}
	return IRQ_NONE;
		
}

static int simple_platform_driver_probe (struct platform_device *pdev)
{
	int ret;
	printk(KERN_ALERT " %s\n", __func__);
	ret = request_irq(irq, &simple_interrupt_handler, IRQF_SHARED, "simple-platform-device" , pdev);
	if(ret) {
		printk(KERN_ALERT " Failed in request_irq  %s\n", __func__);
		return -EIO;
	}
	
	return ret;

}

static int simple_platform_driver_remove (struct platform_device *pdev)
{
	printk(KERN_ALERT " %s\n", __func__);
	free_irq(irq, pdev);
	tasklet_kill(&my_tasklet);
	return 0;
}


static struct platform_device simple_device = {
	.name = "simple-platform-device",
	.id = PLATFORM_DEVID_NONE, 
};


static struct platform_driver simple_driver  = {
	.probe = simple_platform_driver_probe,
	.remove = simple_platform_driver_remove,
	.driver = {
	    .name = "simple-platform-device",
	    .owner = THIS_MODULE,
	},
}; 


static int simple_platform_init(void)
{
	printk(KERN_ALERT "welcome in platfrom driver init function\n");
	
	/* Register my platform device with the kernel */
	platform_device_register(&simple_device);
	
	/* Register my platfrom driver with kernel. 
	   We will use this when we are not sure if this device is present in the
	   system. The driver probe will be called by the OS if the device is
	   present in the system. */

	return platform_driver_register(&simple_driver);
	
}


static void simple_platform_exit(void)
{
	printk(KERN_ALERT "exit %s\n", __func__);
	platform_driver_unregister(&simple_driver);
	platform_device_unregister(&simple_device);
}

module_init(simple_platform_init);
module_exit(simple_platform_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Namarta Kohli");


