#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

static int major = 250;
static int minor = 0;
static dev_t devno;
static struct class *cls;
static struct device *test_device;

static int test_open(struct inode *inode, struct file *filep)
{
	printk(KERN_ALERT "open dev\n");
	return 0;
}

static struct file_operations test_ops = 
{
	.open = test_open,
};

static int __init test_init(void)
{
	int ret;

	devno = MKDEV(major, minor);
	ret = register_chrdev(major, "test", &test_ops);
	if (ret)
		return -1;

	cls = class_create(THIS_MODULE, "test_class");
	if (IS_ERR(cls))
	{
		unregister_chrdev(major, "test");
		return -EBUSY;
	}

	test_device = device_create(cls, NULL, devno, NULL, "test");
	if (IS_ERR(test_device))
	{
		class_destroy(cls);
		unregister_chrdev(major, "test");
		return -EBUSY;
	}
	printk(KERN_ALERT "test_init\n");
	
	return 0;
}

static void test_exit(void)
{
	device_destroy(cls, devno);
	class_destroy(cls);
	unregister_chrdev(major, "test");
	printk(KERN_ALERT "test_exit\n");
}

MODULE_AUTHOR("zhangyinjun");
MODULE_DESCRIPTION("test app");
MODULE_LICENSE("GPL");

module_init(test_init);
module_exit(test_exit);
