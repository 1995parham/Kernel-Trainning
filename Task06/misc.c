/*
 * In The Name Of God
 * ========================================
 * [] File Name : misc.c
 *
 * [] Creation Date : 29-12-2014
 *
 * [] Last Modified : Thu 08 Jan 2015 07:31:57 PM IRST
 *
 * [] Created By : Parham Alvani (parham.alvani@gmail.com)
 * =======================================
*/
#include <linux/module.h>
#include <linux/init.h>

#include <linux/kernel.h>       /* printk() */
#include <linux/string.h>	/* strlen() */
#include <linux/fs.h>           /* everything... */
#include <linux/errno.h>        /* error codes */
#include <linux/types.h>        /* size_t */
#include <linux/cdev.h>
#include <asm/uaccess.h>        /* copy_*_user */

/*
 * Parameters which can be set at load time.
*/
int misc_major = 0;
int misc_minor = 0;

struct cdev dev;

MODULE_AUTHOR("Parham Alvani");
MODULE_LICENSE("GPL");

/*
 * Data management: read and write
*/
ssize_t misc_read(struct file *filp, char __user *buf, size_t count,
		loff_t *f_pos)
{
	return simple_read_from_buffer(buf, count, f_pos,
			"8d7990499d47\n", strlen("8d7990499d47\n"));
}

ssize_t misc_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *f_pos)
{
	int retval = 0;
	const char idstr[] = "8d7990499d47";
	const int idlen = strlen(idstr);
	char input_id[idlen + 1];

	if (copy_from_user(input_id, buf, idlen)) {
		retval = -EFAULT;
		goto out;
	}
	input_id[idlen] = 0;

	if (strcmp(idstr, input_id))
		retval = -EINVAL;
	else
		retval = count;
out:
	return retval;
}

const struct file_operations misc_fops = {
	.owner =	THIS_MODULE,
	.read =		misc_read,
	.write =	misc_write,
};

/*
 * Finally, the module stuff
*/

void misc_setup_cdev(void)
{
	int err;
	dev_t devno = MKDEV(misc_major, misc_minor);

	cdev_init(&dev, &misc_fops);
	dev.owner = THIS_MODULE;
	dev.ops = &misc_fops;
	err = cdev_add(&dev, devno, 1);
	if (err)
		pr_warn("MISC: error %d adding lifo", err);
	else
		pr_info("MISC: cdev registeration completed");
}

/*
 * The cleanup function is used to handle initialization failures as well.
 * Thefore, it must be careful to work correctly even if some of the items
 * have not been initialized
*/
void __exit misc_cleanup_module(void)
{
	dev_t devno = MKDEV(misc_major, misc_minor);

	cdev_del(&dev);

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, 1);
}

int __init misc_init_module(void)
{
	int result;
	dev_t devno = 0;

	result = alloc_chrdev_region(&devno, misc_minor, 1, "misc");
	misc_major = MAJOR(devno);

	if (result < 0) {
		pr_warn("MISC: can't get major %d\n", misc_major);
		return result;
	}

	pr_info("MISC: major: %d , minor %d allocated\n",
			misc_major, misc_minor);

	misc_setup_cdev();

	return 0;
}

module_init(misc_init_module);
module_exit(misc_cleanup_module);
