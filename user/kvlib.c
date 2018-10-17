/**
 * User space library implementation. The library is used by user-space 
 * processes wanting to manipulate the key value-store. It provides high-level,
 * easy to use functions that abstracts the IOCTL interface with the kernel 
 * module through the virtual device
 */

/* FIXME: RO mode ret code for set_keycal and get_keyval */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

/* here we get some info from the virtual device header: device name, major 
 * number, ioctl commands identifiers, and the struct keyval definition */
#include "../kernel/device.h"

/**
 * Called by a process wanting to do a format operation.
 * Returns:
 * 0 on success
 * -1 on error when opening the virtual device file
 * -2 on ioctl error
 * -3 on erase operation error
 */
int kvlib_format()
{
	int fd, ret;

	/* open virtual device file */
	fd = open(DEVICE_NAME, 0);
	if (fd < 0)
		return -1; /* error openning device file */

	/* send ioclt command */
	if (ioctl(fd, IOCTL_FORMAT, &ret) != 0)
		ret = -2; /* ioctl error */
	else if (ret != 0)
		ret = -3; /* error during driver erase operation */

	/* close virtual device */
	close(fd);
	return ret;
}

/**
 * Called by a process wanting to write a key/value couple (set)
 * Returns:
 * 0 on success
 * -1 on error when opening the virtual device file
 * -2 on IOCTL error
 * -3 if size(key + value) > flash page size
 * -4 when trying to set an already existing key
 * -5 when the storage system is in read-only mode
 * -6 on MTD write error
 * -7 on userspace/kernel space memory transfer error
 */
int kvlib_set(const char *key, const char *value)
{
	int fd;
	int ret = 0;
	keyval kv;

	/* open virtual device file */
	fd = open(DEVICE_NAME, 0);
	if (fd < 0)
		return -1;
	
	/* prepare the keyval structure that will be sent through ioctl */
	kv.key = (char *)malloc((strlen(key) + 1) * sizeof(char));
	kv.val = (char *)malloc((strlen(value) + 1) * sizeof(char));
	sprintf(kv.key, "%s", key);
	sprintf(kv.val, "%s", value);

	kv.key_len = strlen(key);
	kv.val_len = strlen(value);

	/* send ioctl command */
	if (ioctl(fd, IOCTL_SET, &kv) != 0) {
		return -2; /* ioctl error */
	} else {
		/* get the return code */
		if (kv.status == -1) {
			ret = -3; /* size to write too big */
		} else if (kv.status == -2) {
			ret = -4; /* key already exists */
		} else if (kv.status == -3) {
			ret = -5; /* system in RO mode */
		} else if (kv.status == -4) {
			ret = -6; /* MTD write error */
		}
	}

	/* cleanup */
	free(kv.key);
	free(kv.val);

	/* close virtual device file */
	close(fd);
	return ret;
}

/**
 * Called by a process to get a value from a key
 * Returns:
 * 0 when ok
 * -1 on virtual device file open error
 * -2 on IOCTL error
 * -3 if key not found
 * -4 on flash read error
 * -5 on user/kernelspace memory transfer error
 */
int kvlib_get(const char *key, char *value)
{
	int fd;
	int ret = 0;
	keyval kv;

	/* open virtual device file */
	fd = open(DEVICE_NAME, 0);
	if (fd < 0)
		return -1;

	/* peprare the keyval structure we will send through IOCTL */
	kv.key = (char *)malloc((strlen(key) + 1) * sizeof(char));
	kv.val = (char *)malloc((strlen(value) + 1) * sizeof(char));
	sprintf(kv.key, "%s", key);
	kv.key_len = strlen(key);

	/* ioctl */
	if (ioctl(fd, IOCTL_GET, &kv) != 0)
		return -2; /* ioctl error */

	/* get the value... */
	sprintf(value, "%s", kv.val);

	/* ... and the return code */
	if (kv.status == -1)
		ret = -3; /* key not found */
	else if (kv.status == -2)
		ret = -4; /* flash read error */

	free(kv.key);
	free(kv.val);

	close(fd);

	return ret;
}

/**
 * Delete a key
 * Returns:
 * 0 when ok
 * -1 on virtual device file open error
 * -2 on ioctl error
 * -3 if key does not exist
 * -4 on user/kernelspace memory transfer error 
 */
int kvlib_del(const char *key)
{
    int fd;
    int ret = 0;
    keyval kv;

    fd = open(DEVICE_NAME, 0);
    if (fd < 0)
        return -1;

    kv.key = (char *)malloc((strlen(key) + 1) * sizeof(char));
    sprintf(kv.key, "%s" ,key);
    kv.key_len = strlen(key);

    /* perform IOCTL */
    if (ioctl(fd, IOCTL_DEL, &kv) != 0)
        return -2;

    /* set return codes */
    if (kv.status == -1)
        ret = -3;   /* key not found */
    else if (kv.status == -2)
        ret = -4;   /* user/kernelspace memory transfer error */

    free(kv.key);

    close(fd);
    
    return ret;
}


/**
 * Called by a process to update a key/value pair
 * Returns:
 * 0 on success
 * -1 on error when opening the virtual device file
 * -2 on IOCTL error
 * -3 if size(key + value) > flash page size
 * -4 if key does not exist
 * -5 when the storage system is in read-only mode
 * -6 on MTD write error
 * -7 on userspace/kernel space memory transfer error
 */
int kvlib_update(const char *key, const char *value)
{
	int fd;
	int ret = 0;
	keyval kv;

	/* open virtual device file */
	fd = open(DEVICE_NAME, 0);
	if (fd < 0)
		return -1;
	
	/* prepare the keyval structure */
	kv.key = (char *)malloc((strlen(key) + 1) * sizeof(char));
	kv.val = (char *)malloc((strlen(value) + 1) * sizeof(char));
	sprintf(kv.key, "%s", key);
	sprintf(kv.val, "%s", value);

	kv.key_len = strlen(key);
	kv.val_len = strlen(value);

	/* send ioctl command */
	if (ioctl(fd, IOCTL_UPDATE, &kv) != 0) {
		return -2; /* ioctl error */
	} else {
		/* get the return code */
		if (kv.status == -1) {
			ret = -3; /* size to write too big */
		} else if (kv.status == -2) {
			ret = -4; /* key does not exist */
		} else if (kv.status == -3) {
			ret = -5; /* system in RO mode */
		} else if (kv.status == -4) {
			ret = -6; /* MTD write error */
		} else if (kv.status == -5) {
            ret = -7; /* mem transfer error */
        }
	}

	/* cleanup */
	free(kv.key);
	free(kv.val);

	/* close virtual device file */
	close(fd);
	return ret;
}


