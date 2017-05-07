/**
 * Test program using the library to access the storage system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Library header */
#include "kvlib.h"

#define PRINT_PREF  "[TESTBENCH]: "

int test_set(const char *key, const char *val);
int test_get(const char *key, const char *val);
int test_delete(const char *key);
int test_format();
int test_update(const char *key, const char *val);

int test_set(const char *key, const char *val) {
    int ret = 0;

    ret = kvlib_set(key, val);
    if (ret != 0) {
        printf("Set (kvlib_set) failed with inputs %s and %s. Returned value: %d\n", key, val, ret);
        return -1;
    }
    return 0;
}

int test_get(const char *key, const char *expected_val) {
    char buffer[128];
    int ret = 0;
    
    ret = kvlib_get(key, buffer);
    if (ret != 0) {
        printf("Get (kvlib_get) failed for key: %s, expected value: %s, Received value: %s. Error code: %d\n", key, expected_val, buffer, ret);
        return -1;
    }
    return 0;
}

int test_format() {
    int ret = 0;
    
    ret = kvlib_format();
    if (ret != 0) {
        printf("Format (kvlib_format) failed. Returned value: %d\n", ret);
        return -1;
    }
    return 0;
}

int test_delete(const char *key) {
    int ret = 0;

    ret = kvlib_del(key);
    if (ret != 0) {
        printf("Delete (kvlib_del) failed for key: %s. Returned value: %d\n", key, ret);
        return -1;
    }
    return 0;
}

int test_update(const char *key, const char *val) {
    int ret = 0;
    
    ret = kvlib_update(key, val);
    if (ret != 0) {
        printf("Update (kvlib_update) failed for key: %s, val:%s. Returned value: %d\n", key, val, ret);
        return -1;
    }
    return 0;
}




int main(void)
{
	//return persistenceWrite();  // comment this to lauch read
	//return persistenceRead();
	int ret, i;
	char buffer[128];
	buffer[0] = '\0';
    int errors = 0;

	/* first let's format the partition to make sure we operate on a 
	 * well-known state */
    errors += test_format();

    /* "set" operation test */
	printf("\n\n\n*********Testing set:\n");

    errors += test_set("key1", "val1");
	printf("Insert 1 (key1, val1):\n");
	
	errors += test_set("key2", "val2");
	printf("Insert 2 (key2, val2):\n");


    /* Now let's fill an entire block lplus an additional page (we assume 
	 * there are 64 pages per block) */
	ret = 0;
	for (i = 3; i < 65; i++) {
		char key[128], val[128];
		sprintf(key, "key%d", i);
		sprintf(val, "val%d", i);
		errors += test_set(key, val);
	}
	printf("Insert 3 to 65:\n");

    if (errors != 0) {
        printf(PRINT_PREF " Errors during insertion. errors = %d", errors);
        return -1;
    }

	/* "get" operation test */
	printf("\n\n\n*********Testing get: reading keys 1 to 64\n");
    
	for (i = 1; i < 65; i++) {
		char key[128], val[128];
		sprintf(key, "key%d", i);
		sprintf(val, "val%d", i);
		errors += test_get(key, val);
	}
    
    if (errors != 0){
        printf(PRINT_PREF "Failed Get. Errors = %d\n", errors);
        return -1;
    }

	printf("************Passed GET\n");


    /************************ Update test *******************************/

	printf("\n\n\n*********Testing update:\n");
    
    for (i = 1; i < 65; i++) {
		char key[128], val[128];
		sprintf(key, "key%d", i);
		sprintf(val, "val%d_updated", i);
		errors += test_update(key, val);
	}

    if (errors != 0) {
        printf(PRINT_PREF "Failed Update. Errors = %d\n", errors);
        return -1;
    }



    /************************ Delete test *******************************/
	printf("\n\n\n*********Testing del:\n");
    for (i = 1; i < 65; i++) {
		char key[128];
		sprintf(key, "key%d", i);
		errors += test_delete(key);
	}

    if (errors != 0) {
        printf(PRINT_PREF "Failed Delete. Errors = %d\n", errors);
        return -1;
    }


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
