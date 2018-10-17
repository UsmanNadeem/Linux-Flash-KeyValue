/**
 * This file contains the code related to the virtual device
 */

#include "device.h"

#include <linux/fs.h>
#include <linux/ioctl.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>

#include "core.h"

/* global attributes for the virtual device */
static atomic_t file_is_open;

/**
 * called when a process opens the virtual device file 
 * i.e. open("/dev/lkp_kv")
 */
static int device_open(struct inode *inode, struct file *file)
{
	/* only a single process can open the device at a time */
	int is_open = atomic_read(&file_is_open);
	if (is_open)
		return -EBUSY;

	atomic_set(&file_is_open, 1);

	return 0;
}

/**
 * function called when the virtual device file is closed
 */
static int device_release(struct inode *inode, struct file *file)
{
	atomic_set(&file_is_open, 0);
	return 0;
}

/**
 * ioctl reception. In simplicity order, first study format, then get, 
 * then set
 */
static long device_ioctl(struct file *file, unsigned int ioctl_num,
			 unsigned long ioctl_param)
{
	switch (ioctl_num) {
		/* format operation */
	case IOCTL_FORMAT:
		{
			int ret;
			/* call module core function */
			ret = format();

			/* copy return code to userspace */
			put_user(ret, (int *)ioctl_param);
			break;
		}

		/* set operation */
	case IOCTL_SET:
		{
			int ret = 0;
			int err_bytes_copied = 0;
			char *key, *val;
			keyval kv;

			/* get the keyval structure from userspace
			 * warning: character pointers in the struct still point
			 * to userspace data */
			err_bytes_copied +=
			    copy_from_user(&kv, (void *)ioctl_param,
					   sizeof(keyval));

			key = (char *)vmalloc((kv.key_len + 1) * sizeof(char));
			val = (char *)vmalloc((kv.val_len + 1) * sizeof(char));
			/* now that we have the character string sizes, we get get the
			 * string themselves */
			err_bytes_copied +=
			    copy_from_user(key, kv.key, kv.key_len + 1);
			err_bytes_copied +=
			    copy_from_user(val, kv.val, kv.val_len + 1);

			if (!err_bytes_copied)
				ret = set_keyval(key, val);	/* call module core function */
			else
				ret = -7;

			/* copy return code to userspace */
			put_user(ret,
				 (int *)&(((keyval *) (ioctl_param))->status));

			/* nettoyage */
			vfree(val);
			vfree(key);
			break;
		}

		/* get operation */
	case IOCTL_GET:
		{
			int ret = 0;
			int err_bytes_copied = 0;
			char *key, *val;
			keyval kv;

			/* get the keyval struct */
			err_bytes_copied +=
			    copy_from_user(&kv, (void *)ioctl_param,
					   sizeof(keyval));

			key = (char *)vmalloc((kv.key_len + 1) * sizeof(char));
			val =
			    (char *)vmalloc((config.page_size) * sizeof(char));

			/* get the key */
			err_bytes_copied +=
			    copy_from_user(key, kv.key, kv.key_len + 1);

			if (!err_bytes_copied) {
				ret = get_keyval(key, val);	/* appel au coeur du module */
				if (ret >= 0) {
					/* write the result to userspace */
					err_bytes_copied +=
					    copy_to_user(kv.val, val,
							 strlen(val) + 1);
				}
			}

			if (err_bytes_copied)
				ret = -5;

			/* copy return code to userspace */
			put_user(ret,
				 (int *)&(((keyval *) (ioctl_param))->status));

			vfree(val);
			vfree(key);

			break;
		}

        /* delete operation */
        /* return values: 
         * 0  : success
         * -1 : key not found
         * -2 : user/kernelspace mem transfer error */
    case IOCTL_DEL:
        {
            int ret = 0;
            int err_bytes_copied = 0;
            char *key;
            keyval kv;
            
            /* get the keyval structure from userspace */
            err_bytes_copied += copy_from_user(&kv, (void *)ioctl_param,
                    sizeof(keyval));
            
            /* get key */
            key = (char *)vmalloc((kv.key_len + 1) * sizeof(char));
            err_bytes_copied += copy_from_user(key, kv.key, kv.key_len + 1);

            if (!err_bytes_copied)
                ret = del_keyval(key);  /* call the module function to delete*/
            else
                ret = -2;   /* user/kernelspace memory transfer error */

            /* put return code to userspace */
            put_user(ret, (int *)&(((keyval *) (ioctl_param))->status));

            /* free the buffer */
            vfree(key);
            break;
        }

        /* update operation */
    case IOCTL_UPDATE:
        {
            int ret = 0;
            int err_bytes_copied = 0;
            char *key, *val;
            keyval kv;

            /* read keyval struct from userspace*/
            err_bytes_copied = copy_from_user(&kv, (void *)ioctl_param, 
                    sizeof(keyval));
            
            /* read key and value */
            key = (char *)vmalloc((kv.key_len + 1) * sizeof(char));
            val = (char *)vmalloc((kv.val_len + 1) * sizeof(char));

            err_bytes_copied += copy_from_user(key, kv.key, kv.key_len + 1);
            err_bytes_copied += copy_from_user(val, kv.val, kv.val_len + 1);
            
            if (!err_bytes_copied)
                ret = update_keyval(key, val);
            else
                return -5;  /* user/kernel space mem transfer error */ 
            
            /* copy ret code to userspace */
            put_user(ret, (int *)&(((keyval *) ioctl_param)->status));

            /*free buffers*/
            vfree(key);
            vfree(val);
            
            break;
        }

	default:
		return -8;	/* bad ioctl code */
	}

	return 0;
}

/* functions to manipulate the virtual device file */
struct file_operations Fops = {
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release,
};

/* Virtual device initialization, called from the module init function
 */
int device_init(void)
{
	int ret;

	atomic_set(&file_is_open, 0);

	/* virtual device creation */
	ret = register_chrdev(MAJOR_NUM, DEVICE_NAME, &Fops);
	if (ret < 0)
		return -1;

	return 0;
}

/**
 * virtual device deletion, called from the module exit function
 */
void device_exit(void)
{
	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
}
