#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/device.h>
#include<linux/interrupt.h>
#include<linux/platform_device.h>
#include<linux/kthread.h> //kernel thread
#include<linux/sched.h> //task struct 
#include<linux/delay.h> //msleep
#include<linux/spinlock.h>

/*you can see these kernel thread via ps -ef command*/
static struct task_struct *my_thread;
int irq = 15;
int global_data =0;

static void my_tasklet_handler(unsigned long flag);

/*statically initialize a Spinlock */
DEFINE_SPINLOCK(ext_spinlock);

/*statically initialize a Tasklet*/
static DECLARE_TASKLET(my_tasklet, my_tasklet_handler, 156);


static void my_tasklet_handler(unsigned long flag)
{
	int index =0;
	for(; index<=5; index++)
	{	
		printk(KERN_INFO " inside %s with data %lu index = %d\n", __func__, flag, index);
        	spin_lock_bh(&ext_spinlock);
        	printk(KERN_INFO "In tasklet handler global_data =  %d\n", global_data);
		global_data++;
	        spin_unlock_bh(&ext_spinlock);
	}
	return;
}

static irqreturn_t interrupt_handler(int irq, void *dev)
{
	int count = 1;
	if(count ==1)
	{
		printk(KERN_INFO " Interrupt Occured on  IRQ = %d\n", irq);
		tasklet_schedule(&my_tasklet);
		count++;
	}
	printk(KERN_INFO " return from %s \n", __func__);
	
	/* reason to return IRQ_NONE, I want proper driver should handle this*/
	/* I have put an hack there so that's why IRQ_NONE*/
	return IRQ_NONE;
	
}

int thread_function(void * data )
{

	  while(!kthread_should_stop()) {
        	spin_lock_bh(&ext_spinlock);
	        global_data++;
        	printk(KERN_INFO "In Thread Function global_data =  %d\n", global_data);
	        spin_unlock_bh(&ext_spinlock); 
        	msleep(1000);
    	}
    	return 0;

	
	
}
static int simple_platform_driver_probe (struct platform_device *pdev)
{
	printk(KERN_ALERT " %s\n", __func__);

	if(request_irq(irq, interrupt_handler, IRQF_SHARED,"simple-platform-device", pdev))
	{
		printk(KERN_INFO "Failed to allocated interrupt on irq number = %d\n", irq);
		return -EIO;
	}

	my_thread= kthread_run(thread_function, NULL, "my_thread");
	if(my_thread)
		printk(KERN_INFO "my kernel thread created sucessfully\n");
	else
		printk(KERN_ERR "faled to create my kernel thread %s\n", __func__);

	
	return 0; 

}

static int simple_platform_driver_remove (struct platform_device *pdev)
{
	printk(KERN_ALERT " %s\n", __func__);
	kthread_stop(my_thread);
	tasklet_kill(&my_tasklet);
	free_irq(irq,pdev);
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


