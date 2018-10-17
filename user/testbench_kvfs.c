/**
 * Test program using the library to access the storage system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Library header */
#include "kvlib.h"

#define PRINT_PREF  "[ TESTBENCH ]: "
#define KGRN  "\x1B[32m"
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"

int test_set(const char *key, const char *val, int expected_return);
int test_get(const char *key, const char *val, int expected_return);
int test_delete(const char *key);
int test_format();
int test_update(const char *key, const char *val);

int test_set(const char *key, const char *val, int expected_return) {
    int ret = 0;

    ret = kvlib_set(key, val);
    if (ret != expected_return) {
        printf(PRINT_PREF "Set (kvlib_set) failed with inputs %s and %s."
                "Expected return value: %d, Returned value: %d\n", 
                key, val, expected_return, ret);
        return -1;
    }
    return 0;
}

int test_get(const char *key, const char *expected_val, int expected_return) {
    char buffer[128];
    int ret = 0;
    
    ret = kvlib_get(key, buffer);
    if (ret != expected_return) {
        printf(PRINT_PREF "Get (kvlib_get) failed for key: %s, expected value: " 
                "%s, Received value: %s. Expected return value: %d Error code: " 
                "%d\n", key, expected_val, buffer, expected_return, ret);
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
    char buffer[128];

    ret = kvlib_del(key);
    if (ret != 0) {
        printf(PRINT_PREF "Delete (kvlib_del) failed for key: %s. Returned value: %d\n", key, ret);
        return -1;
    }
    ret = kvlib_get(key, buffer);
    if (ret != -3) {
        printf(PRINT_PREF "ERROR: Key (kvlib_del) was not deleted. Key: %s\n", key);
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
    printf(KNRM);
	/* first let's format the partition to make sure we operate on a 
	 * well-known state */
    errors += test_format();

    /************************ Get operation test ****************************/

    /* Now let's fill an entire block lplus an additional page (we assume 
	 * there are 64 pages per block) */
	printf(PRINT_PREF " Testing Set operation. (Insert Keys 1->576):\n");
    ret = 0;
	for (i = 1; i < 576; i++) {
		char key[128], val[128];
		sprintf(key, "key%d", i);
		sprintf(val, "val%d", i);
		errors += test_set(key, val, 0);
	}
    if (errors != 0) {
        printf(PRINT_PREF " Errors during insertion. errors = %d", errors);
        return -1;
    }
    printf(PRINT_PREF KGRN "PASSED: " KNRM "Set operation test (Insert Keys 1->576)\n");


    /************************ Get operation test ****************************/
	
    printf("\n\n\n" PRINT_PREF "Testing Get operation: Reading keys 1 to 576\n");
    
	for (i = 1; i < 576; i++) {
		char key[128], val[128];
		sprintf(key, "key%d", i);
		sprintf(val, "val%d", i);
		errors += test_get(key, val, 0);
	}
    
    if (errors != 0){
        printf(PRINT_PREF "Failed Get. Errors = %d\n", errors);
        return -1;
    }

	printf(PRINT_PREF KGRN "PASSED " KNRM "GET operation (Get Keys 1->576)\n");


    /************************ Update test *******************************/

	printf("\n\n\n" PRINT_PREF "*********Testing Update operation:\n");
    printf(PRINT_PREF "Updatring Keys 1->576\n"); 
    for (i = 1; i < 576; i++) {
		char key[128], val[128];
		sprintf(key, "key%d", i);
		sprintf(val, "val%d_updated", i);
		errors += test_update(key, val);
	}

    if (errors != 0) {
        printf(PRINT_PREF "Failed Update. Errors = %d\n", errors);
        return -1;
    }
    printf(PRINT_PREF KGRN "PASSED: " KNRM "Update operation (Update Keys 1->576)\n");


    /*************** Test for updated values in the flash ***************/

    printf("\n\n\n" PRINT_PREF "Testing Get operation: Reading keys 1 to 576\n");
    
	for (i = 1; i < 576; i++) {
		char key[128], val[128];
		sprintf(key, "key%d", i);
		sprintf(val, "val%d_updated", i);
		errors += test_get(key, val, 0);
	}
    
    if (errors != 0){
        printf(PRINT_PREF "FAILED while reading updated values. Errors = %d\n", errors);
        return -1;
    }

	printf(PRINT_PREF KGRN "PASSED " KNRM "Validated that key-value pairs were" 
            " updated (Get Keys 1->576)\n");


    /************************ Delete test *******************************/
	printf("\n\n\n" PRINT_PREF "*********Testing del:\n");
    printf(PRINT_PREF "Deleting Keys 1->576\n");
    for (i = 1; i < 576; i++) {
		char key[128];
		sprintf(key, "key%d", i);
		errors += test_delete(key);
	}

    if (errors != 0) {
        printf(PRINT_PREF "Failed Delete. Errors = %d\n", errors);
        return -1;
    }
    printf(PRINT_PREF KGRN "PASSED: " KNRM "Delete operation (Delete Keys 1->576)\n");

    /************************ Get deleted keys test *******************************/

    printf("\n\n\n" PRINT_PREF "Testing Get for deleted keys operation: "
            " Reading keys 1 to 576. Should not be able to get keys\n");
    
	for (i = 1; i < 576; i++) {
		char key[128], val[128];
		sprintf(key, "key%d", i);
		sprintf(val, "val%d", i);
		errors += test_get(key, val, -3);
	}
    
    if (errors != 0){
        printf(PRINT_PREF "FAILED: Deleted keys seem to exist. Errors = %d\n", errors);
        return -1;
    }

	printf(PRINT_PREF KGRN "PASSED " KNRM "Deleted keys do not exist (Get Keys 1->576)\n");


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
	printf (PRINT_PREF "Insertion (flash should be full after that)"
		", may take some time...\n");
	fflush(stdout);
	ret = 0;

    /* METADATA takes about 6 pages for the configuration BLOCKS=10,PAGE_PER_BLOCK=64 */
    /* First block is reserved for METADATA, we only have 9 blocks with current config */
    for (i = 0; i < 9; i++) {
        for (j = 0; j < 65; j++) {
            char key[128], val[128];
            sprintf(key, "key%d", i * 65 + j);
            sprintf(val, "val%d", i * 65 + j);
            errors += test_set(key, val, 0);
            errors += test_get(key, val, 0);
            if (j < (2 * i + 1)){
                //printf(PRINT_PREF "Deleting Key: %s\n", key);
                errors += test_delete(key);
            }
        }
        /* Delete some pages in this block, later blocks will have more
         * deleted pages and more chances of being selected for GC. */
    }

    if (errors != 0) {
        printf(PRINT_PREF "Failed in filling flash during GC test. Errors = %d\n", errors);
        return -1;
    }
    printf(PRINT_PREF KGRN "PASSED: " KNRM "Flash fill operations during GC test successful\n");

    /* Test that all the inserted values are in the Flash */
    for (i = 0; i < 9; i++) {
        for (j = 0; j < 65; j++) {
            char key[128], val[128];
            if (j >= (2 * i + 1)) {
                sprintf(key, "key%d", i * 65 + j);
                sprintf(val, "val%d", i * 65 + j);
                //printf(PRINT_PREF "Trying to get key:  %s\n", key);
                errors += test_get(key, val, 0);
            }
            else {
                //sprintf(key, "key%d", i * 65 + j);
                //printf(PRINT_PREF "Ignoring keys: %s\n", key);
            }
        }
    }

    if (errors != 0) {
        printf(PRINT_PREF "Failed in validating flash state after GC. Errors = %d\n", errors);
        return -1;
    }
    printf(PRINT_PREF KGRN "PASSED: " KNRM "Flash state successfully validated after GC\n");
    
    /* Fill the flash completely */
    
    for (i = 1000; i < 1072; i++) {
        char key[128], val[128];
        sprintf(key, "key%d", i);
        sprintf(val, "val%d", i);
        errors += test_set(key, val, 0);
    }
    if (errors != 0) {
        printf(PRINT_PREF "Failed in completely filling the flash. Errors = %d\n", errors);
        return -1;
    }
    printf(PRINT_PREF KGRN "PASSED: " KNRM "Flash completely full\n");
 

    /* Flash is full and in read-only mode, These writes should fail with ret=-5*/
    for (i = 1072; i < 1080; i++) {
        char key[128], val[128];
        sprintf(key, "key%d", i);
        sprintf(val, "val%d", i);
        errors += test_set(key, val, -5);
    }
    if (errors != 0) {
        printf(PRINT_PREF "Flash should be in RO mode here. No writes allowed."
                "Errors = %d\n", errors);
        return -1;
    }
    printf(PRINT_PREF KGRN "PASSED: " KNRM "Writes in RO mode unsuccessful\n");
    
    
    /* Now, delete 5 keys and try to insert 5 keys */
    for (i = 1000; i < 1005; i++) {
        char key[128];
        sprintf(key, "key%d", i);
        errors += test_delete(key);       
    }
    if (errors != 0) {
        printf(PRINT_PREF "The keys should have been deleted successfully.."
                "Errors = %d\n", errors);
        return -1;
    }
    printf(PRINT_PREF KGRN "PASSED: " KNRM "Deletes in a full flash successfull\n");
    /* Let's insert and full the flash again*/
    for (i = 10000; i < 10005; i++) {
        char key[128], val[128];
        sprintf(key, "key%d", i);
        sprintf(val, "val%d", i);
        errors += test_set(key, val, 0);       
    }
    if (errors != 0) {
        printf(PRINT_PREF "The keys should have been inserted successfully.."
                "Errors = %d\n", errors);
        return -1;
    }
    printf(PRINT_PREF KGRN "PASSED: " KNRM "Garbage collector freed up some" 
            " space as a result of these delete operations." 
            " Writes are now successfull. Flash is full again\n");



    /* Flash was full. Do some deletes so that valid pages in two blocks
     * can fill up a free block. After this deletion, GC will move data
     * from more than 1 blocks into a free block.  */
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 65; j++) {
            char key[128];
            if (j >= (2 * i + 1) && j <= 40) {
                sprintf(key, "key%d", i * 65 + j);
                errors += test_delete(key);
            }
        }
    }

    if (errors != 0) {
        printf(PRINT_PREF KRED "FAILED:" KNRM " Could not delete some keys." 
                " Errors = %d\n", errors);
        return -1;
    }
    printf(PRINT_PREF KGRN "PASSED: " KNRM "Successfully freed up some space" 
            " in the flash. Note that the GC will free up more that one blocks"
             " during garbage collection.\n");
    

    /* Let's insert some keys so that GC is invoked */
    for (i = 20000; i < 20010; i++) {
        char key[128], val[128];
        sprintf(key, "key%d", i);
        sprintf(val, "val%d", i);
        errors += test_set(key, val, 0);       
    }
    if (errors != 0) {
        printf(PRINT_PREF KRED "FAILED:" KNRM " The keys should have been"
                " inserted successfully. Errors = %d\n", errors);
        return -1;
    }
    printf(PRINT_PREF KGRN "PASSED: " KNRM "Deletes had freed up some space." 
            " Writes are now successfull.\n");



	/* Format again */
	ret = kvlib_format();
	printf(PRINT_PREF "Formatting:\n");
	printf(PRINT_PREF " returns: %d (should be 0)\n", ret);

	return EXIT_SUCCESS;

}
