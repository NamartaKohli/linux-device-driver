#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/device.h>
#include<linux/platform_device.h>
#include<linux/kthread.h> //kernel thread
#include<linux/sched.h> //task struct 
#include<linux/delay.h> //msleep

/*you can see these kernel thread via ps -ef command*/

static struct task_struct *short_work_thread, *long_work_thread;
int data = 10;

static int short_work_thread_function(void *data);
static int long_work_thread_function(void *data);

int short_work_thread_function(void *data)
{
	int index =0;
	for(index =0; index<5; index++)
		printk(KERN_ALERT "%s index = %d\n", __func__, index);

	/*threadfn() can either call do_exit() directly if it is a
	  standalone thread for which no one will call kthread_stop() */
	do_exit(0);
	return 0;
}
int long_work_thread_function(void *data)
{
	int index = *(int*)data; 

	/*If that thread is a long run thread, we need to check kthread_should_stop() 
	  every time as because any function may call kthread_stop. If any function called kthread_stop, 
	  that time kthread_should_stop will return true. We have to exit our thread function if 
	  true value been returned by kthread_should_stop.*/
	while(!kthread_should_stop()) {
		printk(KERN_ALERT "%s index = %d\n", __func__, index++);
		msleep(1000);	
	}
	return 0;
	
}

static int simple_platform_driver_probe (struct platform_device *pdev)
{
	printk(KERN_ALERT " %s\n", __func__);
	short_work_thread = kthread_create(&short_work_thread_function, NULL, "short_thread");
	if(short_work_thread)
		wake_up_process(short_work_thread);
	else
		printk(KERN_ERR "faled to create short kernel thread %s\n", __func__);
	
	long_work_thread= kthread_run(long_work_thread_function, &data, "long_thread");
	if(long_work_thread)
		printk(KERN_INFO "long kthread created sucessfully\n");
	else
		printk(KERN_ERR "faled to create long kernel thread %s\n", __func__);

	
	return 0; 

}

static int simple_platform_driver_remove (struct platform_device *pdev)
{
	kthread_stop(long_work_thread);
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


