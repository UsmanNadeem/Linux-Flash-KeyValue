/**
 * Test program using the library to access the storage system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Library header */
#include "kvlib.h"

int persistenceWrite(void) {
	int ret, i;

	/* first let's format the partition to make sure we operate on a 
	 * well-known state */
	// ret = kvlib_format();
	// printf("Formatting done:\n");
	// printf(" returns: %d (should be 0)\n", ret);

	/* "set" operation test */
	printf("\n\n\n*********Testing set:\n");
	ret = kvlib_set("key1", "val1");
	//printf("Insert 1 (key1, val1):\n");
	//printf(" returns: %d (should be 0)\n", ret);

	ret = kvlib_set("key2", "val2");
	//printf("Insert 2 (key2, val2):\n");
	//printf(" returns: %d (should be 0)\n", ret);

	/* Now let's fill an entire block lplus an additional page (we assume 
	 * there are 64 pages per block) */
	ret = 0;
	for (i = 3; i < 65; i++) {
		char key[128], val[128];
		sprintf(key, "key%d", i);
		sprintf(val, "val%d", i);
		ret += kvlib_set(key, val);
	}
	// printf("Insert 3 to 64:\n");
	// printf(" returns: %d (should be 0)\n", ret);
	return ret == 0  ?	EXIT_SUCCESS : EXIT_FAILURE;
}

int persistenceRead(void)
{
	int ret, i;
	char buffer[128];
	buffer[0] = '\0';

	

	/* "get" operation test */
	printf("\n\n\n*********Testing get: reading keys 1 to 64\n");

	ret = kvlib_get("key1", buffer);
	 printf("Reading the value of key1:\n");
	 printf(" returns: %d (should be 0), read: %s (should be val1)\n", ret,
		 buffer);

	for (i = 1; i < 65; i++) {
		char key[128], val[128];
		sprintf(key, "key%d", i);
		sprintf(val, "val%d", i);
		kvlib_get(key, buffer);
		if (strcmp(val, buffer)) {
			printf("************Failed GET: \n%s:%s\ngot \"%s\"\n", key, val, buffer);
			exit(EXIT_FAILURE);
		}

	}
	 printf("************Passed GET\n");


	return EXIT_SUCCESS;

}



int main(void)
{
	// kvlib_format();
	// return persistenceWrite();  // comment this to lauch read
	return persistenceRead();
	int ret, i;
	char buffer[128];
	buffer[0] = '\0';

	/* first let's format the partition to make sure we operate on a 
	 * well-known state */
	ret = kvlib_format();
	printf("Formatting done:\n");
	printf(" returns: %d (should be 0)\n", ret);

	/* "set" operation test */
	printf("\n\n\n*********Testing set:\n");
	ret = kvlib_set("key1", "val1");
	printf("Insert 1 (key1, val1):\n");
	printf(" returns: %d (should be 0)\n", ret);

	ret = kvlib_set("key2", "val2");
	printf("Insert 2 (key2, val2):\n");
	printf(" returns: %d (should be 0)\n", ret);

	/* Now let's fill an entire block lplus an additional page (we assume 
	 * there are 64 pages per block) */
	ret = 0;
	for (i = 3; i < 65; i++) {
		char key[128], val[128];
		sprintf(key, "key%d", i);
		sprintf(val, "val%d", i);
		ret += kvlib_set(key, val);
	}
	printf("Insert 3 to 64:\n");
	printf(" returns: %d (should be 0)\n", ret);

	/* "get" operation test */
	printf("\n\n\n*********Testing get: reading keys 1 to 64\n");

	ret = kvlib_get("key1", buffer);
	 printf("Reading the value of key1:\n");
	 printf(" returns: %d (should be 0), read: %s (should be val1)\n", ret,
		 buffer);

	//ret = kvlib_get("key35", buffer);
	// printf("Reading the value of key35:\n");
	// printf(" returns: %d (should be 0), read: %s (should be val35)\n", ret,
		// buffer);
	for (i = 1; i < 65; i++) {
		char key[128], val[128];
		sprintf(key, "key%d", i);
		sprintf(val, "val%d", i);
		kvlib_get(key, buffer);
		if (strcmp(val, buffer)) {
			printf("************Failed GET: \n%s:%s\ngot \"%s\"\n", key, val, buffer);
			exit(EXIT_FAILURE);
		}

	}
	 printf("************Passed GET\n");


    /************************ Update test *******************************/

	printf("\n\n\n*********Testing update:\n");

	/* Update Test: trying to set a value for an already existing key */
	ret = kvlib_update("key1", "key1_updated");
	printf("Trying to update a key:\n");
	printf(" returns: %d (should be 0 (successful))\n", ret);

    /* Reading updated value */
	ret = kvlib_get("key1", buffer);
	printf("Reading the value of an updated key:\n");
	printf(" returns: %d (should be 0), read: %s (should be key1_updated)\n", ret, buffer);

	/* Another Update Test: trying to set a value for an already existing key */
	ret = kvlib_update("key1", "key1_updated_again");
	printf("Trying to update a key:\n");
	printf(" returns: %d (should be 0 (successful))\n", ret);

    /* Reading updated value */
	ret = kvlib_get("key1", buffer);
	printf("Reading the value of an updated key:\n");
	printf(" returns: %d (should be 0), read: %s (should be key1_updated_again)\n", ret, buffer);
    
    /* Another Update Test: trying to set a value for an already existing key */
	ret = kvlib_update("key2", "key2_updated");
	printf("Trying to update key2:\n");
	printf(" returns: %d (should be 0 (successful))\n", ret);

    /* Reading updated value */
	ret = kvlib_get("key2", buffer);
	printf("Reading the value of an updated key:\n");
	printf(" returns: %d (should be 0), read: %s (should be key2_updated)\n", ret, buffer);

    /************************ Delete test *******************************/
	printf("\n\n\n*********Testing del:\n");

    /* Delete Test: trying to delete ang key */
	ret = kvlib_del("key1");
	printf("Trying to delete a key:\n");
	printf(" returns: %d (should be 0 (successful))\n", ret);

    /* Reading deleted value */
	ret = kvlib_get("key1", buffer);
	printf("Reading the value of deleted key:\n");
	printf(" returns: %d (should be -3)\n", ret);


    /* Reading deleted key */
	ret = kvlib_del("key2");
	printf("Trying to delete key2:\n");
	printf(" returns: %d (should be 0)\n", ret);

    /* Reading deleted value */
	ret = kvlib_get("key2", buffer);
	printf("Reading the value of deleted key:\n");
	printf(" returns: %d (should be -3)\n", ret);


    /************************ Non-existing key test *******************************/


	/* trying to get the value of a non-existing key */
	ret = kvlib_get("key2000", buffer);
	printf("Trying to get the value of a non-existing key:\n");
	printf(" returns: %d (should be -3)\n", ret);


    /************************ Format again  *******************************/


	/* Let's format again before we fill all the flash */
	ret = kvlib_format();
	printf("Formatting:\n");
	printf(" returns: %d (should be 0)\n", ret);

	/* Fill the flash completely, note that we assume here a small partition
	 * of 10 blocks, each containing 64 pages */
	printf ("Insertion 0->633 (flash should be full after that)"
		", may take some time...\n");
	fflush(stdout);
	ret = 0;

    /*METADATA takes about 6 pages for the configuration BLOCKS=10,PAGE_PER_BLOCK=64*/
	for (i = 0; i < 633; i++) {
		char key[128], val[128];
		sprintf(key, "key%d", i);
		sprintf(val, "val%d", i);
		ret += kvlib_set(key, val);
	}
	printf(" returns: %d (should be 0)\n", ret);

	/* The flash is full and the system should be read-only, let's try to
	 * add an additional key/value: */
	ret = kvlib_set("key634", "val634");
	printf("Trying to insert another key/val:\n");
	printf(" returns: %d (should be -5)\n", ret);

	/* Format again */
	ret = kvlib_format();
	printf("Formatting:\n");
	printf(" returns: %d (should be 0)\n", ret);

	/* Let's try to add a key/value again: */
	ret = kvlib_set("key640", "val640");
	printf("Insert a key/val after formatting:\n");
	printf(" returns: %d (should be 0)\n", ret);

	/* get the value we just inserted */
	ret = kvlib_get("key640", buffer);
	printf("Reading the val of key640:\n");
	printf(" returns: %d, read: %s (should be val640)\n", ret, buffer);

	return EXIT_SUCCESS;

}
