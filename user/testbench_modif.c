/**
 * Test program using the library to access the storage system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Library header */
#include "kvlib.h"

#define PRINT_PREF  "[ TESTBENCH ]: "

int test_set(const char *key, const char *val);
int test_get(const char *key, const char *val);
int test_delete(const char *key);
int test_format();
int test_update(const char *key, const char *val);

int test_set(const char *key, const char *val) {
    int ret = 0;

    ret = kvlib_set(key, val);
    if (ret != 0) {
        printf(PRINT_PREF "Set (kvlib_set) failed with inputs %s and %s. Returned value: %d\n", key, val, ret);
        return -1;
    }
    return 0;
}

int test_get(const char *key, const char *expected_val) {
    char buffer[128];
    int ret = 0;
    
    ret = kvlib_get(key, buffer);
    if (ret != 0) {
        printf(PRINT_PREF "Get (kvlib_get) failed for key: %s, expected value: %s, Received value: %s. Error code: %d\n", key, expected_val, buffer, ret);
        return -1;
    }
    return 0;
}

int test_format() {
    int ret = 0;
    
    ret = kvlib_format();
    if (ret != 0) {
        printf(PRINT_PREF "Format (kvlib_format) failed. Returned value: %d\n", ret);
        return -1;
    }
    return 0;
}

int test_delete(const char *key) {
    int ret = 0;

    ret = kvlib_del(key);
    if (ret != 0) {
        printf(PRINT_PREF "Delete (kvlib_del) failed for key: %s. Returned value: %d\n", key, ret);
        return -1;
    }
    return 0;
}

int test_update(const char *key, const char *val) {
    int ret = 0;
    
    ret = kvlib_update(key, val);
    if (ret != 0) {
        printf(PRINT_PREF "Update (kvlib_update) failed for key: %s, val:%s. Returned value: %d\n", key, val, ret);
        return -1;
    }
    return 0;
}




int main(void)
{
    int ret, i, j;
	char buffer[128];
	buffer[0] = '\0';
    int errors = 0;

	/* first let's format the partition to make sure we operate on a 
	 * well-known state */
    errors += test_format();

    /* "set" operation test */
	printf("\n\n\n" PRINT_PREF "*********Testing set:\n");

    errors += test_set("key1", "val1");
	printf(PRINT_PREF "Insert 1 (key1, val1):\n");
	
	errors += test_set("key2", "val2");
	printf(PRINT_PREF "Insert 2 (key2, val2):\n");


    /* Now let's fill an entire block lplus an additional page (we assume 
	 * there are 64 pages per block) */
	printf(PRINT_PREF " Testing Set operation. (Insert Keys 3->65):\n");
    ret = 0;
	for (i = 3; i < 65; i++) {
		char key[128], val[128];
		sprintf(key, "key%d", i);
		sprintf(val, "val%d", i);
		errors += test_set(key, val);
	}
    if (errors != 0) {
        printf(PRINT_PREF " Errors during insertion. errors = %d", errors);
        return -1;
    }
    printf(PRINT_PREF "Passed Set operation test (Insert Keys 3>65)\n");


    /************************ Get operation test ****************************/
	
    printf("\n\n\n" PRINT_PREF "Testing Get operation: Reading keys 1 to 65\n");
    
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

	printf(PRINT_PREF "************Passed GET operation (Get Keys 1->65)\n");


    /************************ Update test *******************************/

	printf("\n\n\n" PRINT_PREF "*********Testing Update operation:\n");
    printf(PRINT_PREF "Updatring Keys 1->65\n"); 
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
    printf(PRINT_PREF "Update test (Keys 1->65) passed\n");


    /************************ Delete test *******************************/
	printf("\n\n\n" PRINT_PREF "*********Testing del:\n");
    printf(PRINT_PREF "Deleting Keys 1->65\n");
    for (i = 1; i < 65; i++) {
		char key[128];
		sprintf(key, "key%d", i);
		errors += test_delete(key);
	}

    if (errors != 0) {
        printf(PRINT_PREF "Failed Delete. Errors = %d\n", errors);
        return -1;
    }
    printf(PRINT_PREF "Delete test (Keys 1->65) passed\n");

    /************************ Non-existing key test *******************************/


	/* trying to get the value of a non-existing key */
	ret = kvlib_get("key2000", buffer);
	printf(PRINT_PREF "Trying to get the value of a non-existing key:\n");
	printf(PRINT_PREF " returns: %d (should be -3)\n", ret);

    /* Here, test that deleted values don't exist (try to get them) */




    /************************ Format again  *******************************/

	/* Let's format again before we fill all the flash */
	ret = kvlib_format();
	printf(PRINT_PREF "Formatting:\n");
	printf(PRINT_PREF " returns: %d (should be 0)\n", ret);

    
    /************************ Test Garbage Collection Here *******************************/
    
 
	/* Fill the flash completely, note that we assume here a small partition
	 * of 10 blocks, each containing 64 pages */
	printf (PRINT_PREF "Insertion 0->633 (flash should be full after that)"
		", may take some time...\n");
	fflush(stdout);
	ret = 0;

    /* METADATA takes about 6 pages for the configuration BLOCKS=10,PAGE_PER_BLOCK=64 */
    /* First block is reserved for METADATA, we only have 9 blocks with current config */
    for (i = 0; i < 9; i++) {
        for (j = 0; j < 65; j++) {
            char key[128], val[128];
            if (j < 60) {
                sprintf(key, "key%d", i * 64 + j);
                sprintf(val, "val%d", i * 64 + j);
                ret += kvlib_set(key, val);
            }
            else {
                /* last 5 pages will be written by updates, this will 
                 * create 5 invalid pages in the flash block. */
                sprintf(key, "key%d", (i * 64 + j) - 10);
                sprintf(val, "val%d_updated", (i * 64 + j) - 10);
                ret += kvlib_update(key, val);
            }
        }
        /* Delete some pages in this block, later blocks will have more
         * deleted pages and more chances of being selected for GC. */
        for (j = 0; j < (1 + i * 2); j++) {
            char key[128];
            sprintf(key, "key%d", i * 64 + j);           
            ret += kvlib_del(key);
        }
    }

	for (i = 0; i < 576; i++) {
		char key[128], val[128];
		sprintf(key, "key%d", i);
		sprintf(val, "val%d", i);
		ret += kvlib_set(key, val);
	}
	printf(PRINT_PREF " returns: %d (should be 0)\n", ret);

	/* The flash is full and the system should be read-only, let's try to
	 * add an additional key/value: */
	ret = kvlib_set("key634", "val634");
	printf(PRINT_PREF "Trying to insert another key/val:\n");
	printf(PRINT_PREF " returns: %d (should be -5)\n", ret);

	/* Format again */
	ret = kvlib_format();
	printf(PRINT_PREF "Formatting:\n");
	printf(PRINT_PREF " returns: %d (should be 0)\n", ret);

	/* Let's try to add a key/value again: */
	ret = kvlib_set("key640", "val640");
	printf(PRINT_PREF "Insert a key/val after formatting:\n");
	printf(PRINT_PREF " returns: %d (should be 0)\n", ret);

	/* get the value we just inserted */
	ret = kvlib_get("key640", buffer);
	printf(PRINT_PREF "Reading the val of key640:\n");
	printf(PRINT_PREF " returns: %d, read: %s (should be val640)\n", ret, buffer);

	return EXIT_SUCCESS;

}
