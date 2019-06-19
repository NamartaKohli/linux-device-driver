#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/device.h>
#include <linux/platform_device.h>


struct simple_driver {
	struct platform_device *pdev;
	struct regmap *regmap;
	unsigned long paddr;
	struct clk *mem_clk;
	struct clk *ipg_clk;
	struct clk *spba_clk;
	spinlock_t lock;
};




static int simple_platform_driver_probe (struct platform_device *pdev)
{
	struct simple_driver *simple_priv;
	struct resource *res;
	void __iomem *regs;
	int irq, ret, i;

	simple_priv = devm_kzalloc(&pdev->dev, sizeof(*simple_priv), GFP_KERNEL);
	if (!simple_priv)
		return -ENOMEM;

	simple_priv->pdev = pdev;

	/* Get the addresses and IRQ */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	regs = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(regs))
		return PTR_ERR(regs);

	simple_priv->paddr = res->start;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "no irq for node %s\n", pdev->name);
		return irq;
	}

	ret = devm_request_irq(&pdev->dev, irq, simple_driver_isr, 0,
			       dev_name(&pdev->dev), simple_priv);
	if (ret) {
		dev_err(&pdev->dev, "failed to claim irq %u: %d\n", irq, ret);
		return ret;
	}

	simple_priv->mem_clk = devm_clk_get(&pdev->dev, "mem");
	

	return 0;

}

static int simple_platform_driver_remove (struct platform_device *pdev)
{
	printk(KERN_ALERT " %s\n", __func__);
	return 0;
}

static const struct of_device_id simple_device_ids[] = {
	{ .compatible = "platform-device", },
	{}
};



static struct platform_driver simple_driver  = {
	.probe = simple_platform_driver_probe,
	.remove = simple_platform_driver_remove,
	.driver = {
	    .name = "simple-platform-device",
	    .owner = THIS_MODULE,
	    .of_match_table = simple_device_ids,
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


