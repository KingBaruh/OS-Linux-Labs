#include <linux/ctype.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/string.h>

#include "encdec.h"

#define MODULE_NAME "encdec"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YOUR NAME");

int encdec_open(struct inode *inode, struct file *filp);
int encdec_release(struct inode *inode, struct file *filp);
int encdec_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

ssize_t encdec_read_caesar( struct file *filp, char *buf, size_t count, loff_t *f_pos );
ssize_t encdec_write_caesar(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

ssize_t encdec_read_xor( struct file *filp, char *buf, size_t count, loff_t *f_pos );
ssize_t encdec_write_xor(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

int memory_size = 0;

MODULE_PARM(memory_size, "i");

int major = 0;

struct file_operations fops_caesar = {
	.open 	 =	encdec_open,
	.release =	encdec_release,
	.read 	 =	encdec_read_caesar,
	.write 	 =	encdec_write_caesar,
	.llseek  =	NULL,
	.ioctl 	 =	encdec_ioctl,
	.owner 	 =	THIS_MODULE
};

struct file_operations fops_xor = {
	.open 	 =	encdec_open,
	.release =	encdec_release,
	.read 	 =	encdec_read_xor,
	.write 	 =	encdec_write_xor,
	.llseek  =	NULL,
	.ioctl 	 =	encdec_ioctl,
	.owner 	 =	THIS_MODULE
};

// Implemetation suggestion:
// -------------------------
// Use this structure as your file-object's private data structure
typedef struct {
	unsigned char key;
	int read_state;
} encdec_private_data;

char *device_buffer_caesar;
char *device_buffer_xor;


int init_module(void)
{
	// Implemetation suggestion:
	// -------------------------
	// 1. Allocate memory for the two device buffers using kmalloc (each of them should be of size 'memory_size')

	major = register_chrdev(major, MODULE_NAME, &fops_caesar);
	if(major < 0)
	{
		return major;
	}
	device_buffer_caesar = kmalloc(memory_size, GFP_KERNEL);
	device_buffer_xor = kmalloc(memory_size, GFP_KERNEL);
	if (!device_buffer_caesar || !device_buffer_xor) 
	{
		unregister_chrdev(major, MODULE_NAME);
		if (device_buffer_caesar) kfree(device_buffer_caesar);
		if (device_buffer_xor) kfree(device_buffer_xor);
		return -ENOMEM;
	}
	return 0;

}

void cleanup_module(void)
{
	// Implemetation suggestion:
	// -------------------------
	// 1. Unregister the device-driver
	// 2. Free the allocated device buffers using kfree

	unregister_chrdev(major, MODULE_NAME);
	if (device_buffer_caesar) kfree(device_buffer_caesar);
	if (device_buffer_xor) kfree(device_buffer_xor);

}

int encdec_open(struct inode *inode, struct file *filp)
{
	// Implemetation suggestion:
	// -------------------------
	// 1. Set 'filp->f_op' to the correct file-operations structure (use the minor value to determine which)
	// 2. Allocate memory for 'filp->private_data' as needed (using kmalloc)

	int minor = MINOR(inode->i_rdev);

	if (minor == 0)
		filp->f_op = &fops_caesar;
	else if (minor == 1)
		filp->f_op = &fops_xor;
	else /* wrong minor */
		return -EINVAL;

	encdec_private_data* data;
	data = kmalloc(sizeof(encdec_private_data), GFP_KERNEL);
	if (!data) return -ENOMEM;

	data->key = 0;
	data->read_state = ENCDEC_READ_STATE_DECRYPT;
	filp->private_data = data;


	return 0;

}

int encdec_release(struct inode *inode, struct file *filp)
{
	// Implemetation suggestion:
	// -------------------------
	// 1. Free the allocated memory for 'filp->private_data' (using kfree)

	encdec_private_data *data = filp->private_data;
    kfree(data);

	return 0;
}

int encdec_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	// Implemetation suggestion:
	// -------------------------
	// 1. Update the relevant fields in 'filp->private_data' according to the values of 'cmd' and 'arg'

	encdec_private_data* data = filp->private_data;
	switch (cmd) 
	{
	case ENCDEC_CMD_CHANGE_KEY:
		data->key = (unsigned char)arg;
		break;
	case ENCDEC_CMD_SET_READ_STATE:
		data->read_state = (int)arg;
		break;
	case ENCDEC_CMD_ZERO:
		if (filp->f_op == &fops_caesar)
		{
			memset(device_buffer_caesar, 0, memory_size);
		}
		else
		{
			memset(device_buffer_xor, 0, memory_size);
		}
		break;
	default:
		return -ENOTTY;
	}
	return 0;
}

// Add implementations for:
// ------------------------
// 1. ssize_t encdec_read_caesar( struct file *filp, char *buf, size_t count, loff_t *f_pos );
// 2. ssize_t encdec_write_caesar(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
// 3. ssize_t encdec_read_xor( struct file *filp, char *buf, size_t count, loff_t *f_pos );
// 4. ssize_t encdec_write_xor(struct file *filp, const char *buf, size_t count, loff_t *f_pos);


ssize_t encdec_read_caesar(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	int res;
	int i;
	// Ensure we don't read beyond the end of the device buffer
	if (*f_pos >= memory_size)
	{
		return -EINVAL;
	}

	encdec_private_data* data = filp->private_data;
	res = copy_to_user(buf, device_buffer_caesar + (*f_pos), count);

	if (data->read_state == ENCDEC_READ_STATE_DECRYPT)
	{
		for (i = 0; i < count; i++)
		{
			buf[i] = ((buf[i] - data->key) + 128) % 128;
		}
	}
	*f_pos += count - res;
	return count - res;

}

ssize_t encdec_write_caesar(struct file* filp, const char* buf, size_t count, loff_t* f_pos)
{
	int res;
	int i;
	// Ensure we don't write beyond the end of the device buffer
	if (*f_pos >= memory_size) 
	{
		return -ENOSPC;
	}

	encdec_private_data* data = filp->private_data;

	res = copy_from_user(device_buffer_caesar + (*f_pos), buf, count);
	for (i = 0; i < count; i++)
	{
		device_buffer_caesar[i + (*f_pos)] = (device_buffer_caesar[i + (*f_pos)] + data->key) % 128;
	}
	*f_pos += count - res;
	return count - res;

}

ssize_t encdec_read_xor(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	int res;
	int i;
	// Ensure we don't read beyond the end of the device buffer
	if (*f_pos >= memory_size)
	{
		return -EINVAL;
	}

	encdec_private_data* data = filp->private_data;
	res = copy_to_user(buf, device_buffer_xor + (*f_pos), count);

	if (data->read_state == ENCDEC_READ_STATE_DECRYPT)
	{
		for (i = 0; i < count; i++)
		{
			buf[i] = buf[i] ^ data->key;
		}
	}
	*f_pos += count - res;
	return count - res;

}

ssize_t encdec_write_xor(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	int res;
	int i;
	// Ensure we don't write beyond the end of the device buffer
	if (*f_pos >= memory_size)
	{
		return -ENOSPC;
	}

	encdec_private_data* data = filp->private_data;

	res = copy_from_user(device_buffer_xor + (*f_pos), buf, count);
	for (i = 0; i < count; i++)
	{
		device_buffer_xor[i + (*f_pos)] = device_buffer_xor[i + (*f_pos)] ^ data->key;
	}
	*f_pos += count - res;
	return count - res;
}
