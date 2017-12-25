#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>

static int major = 250;
static int minor = 0;
static dev_t devno;
static struct class *cls;
static struct device *test_device;
static unsigned int a = 0;
static struct dentry *dir = NULL, *file_a = NULL, *file_c = NULL;

static int test_open(struct inode *inode, struct file *filep)
{
	printk(KERN_ALERT "open dev\n");
	return 0;
}

static struct file_operations test_ops = 
{
	.open = test_open,
};

static int debugfile_open(struct inode *inode, struct file *filep)
{
	filep->private_data = inode->i_private;
	return 0;
}

static ssize_t debugfile_read(struct file *filep, char __user *buffer, size_t count, loff_t *ppos)
{
	char str[] = "welcome!\n";

	count = strlen(str);
	if (*ppos >= count)
		return 0;

	if (copy_to_user(buffer, str, count))
	{
		return -EFAULT;
	}

	*ppos += count;

	return count;
}

static struct file_operations debugfile_ops =
{
	.open = debugfile_open,
	.read = debugfile_read,
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

	dir = debugfs_create_dir("test_dir", NULL);
	file_a = debugfs_create_u32("test_a", 0644, dir, &a);
	file_c = debugfs_create_file("test_c", 0444, dir, NULL, &debugfile_ops);

	printk(KERN_ALERT "test_init\n");
	
	return 0;
}

static void test_exit(void)
{
	device_destroy(cls, devno);
	class_destroy(cls);
	unregister_chrdev(major, "test");
	debugfs_remove_recursive(dir);
	printk(KERN_ALERT "test_exit[%u]\n", a);
}

MODULE_AUTHOR("zhangyinjun");
MODULE_DESCRIPTION("test app");
MODULE_LICENSE("GPL");

module_init(test_init);
module_exit(test_exit);
