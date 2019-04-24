#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/device.h>
#include <linux/platform_device.h>

static int simple_platform_driver_probe (struct platform_device *pdev)
{
	printk(KERN_ALERT " %s\n", __func__);
	return 0;

}

static int simple_platform_driver_remove (struct platform_device *pdev)
{
	printk(KERN_ALERT " %s\n", __func__);
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


