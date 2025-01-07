#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/etherdevice.h>
#include <net/switchdev.h>

#define TEST_CASE1

#ifdef TEST_CASE0
static int major = 250;
static int minor = 0;
static dev_t devno;
static struct class *cls;
static struct device *test_device;
static unsigned int a = 0;
static struct dentry *dir = NULL;

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

static int test_init_case0(void)
{
	u8 mac[6];
	int ret;

	eth_random_addr(mac);
	printk("mac: [%pM]\n", mac);
	devno = MKDEV(major, minor);
	ret = register_chrdev(major, "test", &test_ops);
	if (ret)
		return ret;

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
	debugfs_create_u32("test_a", 0644, dir, &a);
	debugfs_create_file("test_c", 0444, dir, NULL, &debugfile_ops);
	
	printk(KERN_ALERT "test_init_case0\n");

	return 0;
}

static void test_exit_case0(void)
{
	device_destroy(cls, devno);
	class_destroy(cls);
	unregister_chrdev(major, "test");
	debugfs_remove_recursive(dir);
	printk(KERN_ALERT "test_exit_case0 [%u]\n", a);
}
#endif

#ifdef TEST_CASE1

static int test_netdev_event(struct notifier_block *unused,
			     unsigned long event, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);

	printk("netdev event %lu from dev %s\n", event, dev->name);
	return NOTIFY_DONE;
}

static int test_switchdev_event(struct notifier_block *unused,
				unsigned long event, void *ptr)
{
	struct net_device *dev = switchdev_notifier_info_to_dev(ptr);

	printk("switchdev event %lu from dev %s\n", event, dev->name);
	return NOTIFY_DONE;
}

static int test_switchdev_blk_event(struct notifier_block *unused,
				    unsigned long event, void *ptr)
{
	struct net_device *dev = switchdev_notifier_info_to_dev(ptr);

	printk("switchdev blk event %lu from dev %s\n", event, dev->name);
	return NOTIFY_DONE;
}

static struct notifier_block test_netdev_nb = {
	.notifier_call = test_netdev_event,
};

static struct notifier_block test_switchdev_nb = {
	.notifier_call = test_switchdev_event,
};

static struct notifier_block test_switchdev_blk = {
	.notifier_call = test_switchdev_blk_event,
};

static int test_init_case1(void)
{
	int ret = register_netdevice_notifier(&test_netdev_nb);
	if (ret)
		return ret;

	ret = register_switchdev_notifier(&test_switchdev_nb);
	if (ret) {
		unregister_netdevice_notifier(&test_netdev_nb);
		return ret;
	}

	ret = register_switchdev_blocking_notifier(&test_switchdev_blk);
	if (ret) {
		unregister_switchdev_notifier(&test_switchdev_nb);
		unregister_netdevice_notifier(&test_netdev_nb);
		return ret;
	}

	return 0;
}

static void test_exit_case1(void)
{
	unregister_switchdev_blocking_notifier(&test_switchdev_blk);
	unregister_switchdev_notifier(&test_switchdev_nb);
	unregister_netdevice_notifier(&test_netdev_nb);
}
#endif

static int __init test_init(void)
{
#ifdef TEST_CASE0
	return test_init_case0();
#endif
#ifdef TEST_CASE1
	return test_init_case1();
#endif
	return 0;
}

static void test_exit(void)
{
#ifdef TEST_CASE0
	test_exit_case0();
#endif
#ifdef TEST_CASE1
	test_exit_case1();
#endif
}

MODULE_AUTHOR("zhangyinjun");
MODULE_DESCRIPTION("test app");
MODULE_LICENSE("GPL");

module_init(test_init);
module_exit(test_exit);
