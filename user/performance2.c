/**
 * Test program using the library to access the storage system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/time.h>
/* Library header */
#include "kvlib.h"

#define PRINT_PREF " [Write Performance] "
#define NUM_BLOCKS 9
#define PAGES_PER_BLOCK 64

uint64_t timeW25;
uint64_t timeW50;
uint64_t timeW75;
uint64_t timeW100;
uint64_t timeU25;
uint64_t timeU50;
uint64_t timeU75;
uint64_t timeU100;

int write_to_flash_25(void) {
    int ret, i;
    struct timeval tv;

    /* Write to the flash */
    printf("\n\n\n [Write Performance] Writing to Flash till 25%% capacity:\n\n\n");
    ret = 0;

    gettimeofday(&tv,NULL);
    uint64_t time_in_micros_old = 1000000 * tv.tv_sec + tv.tv_usec;
    uint64_t time_in_micros_new = 0;

    for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS / 4) + 1 ; ++i) {
        char key[128], val[128];
        sprintf(key, "key%d", i);
        sprintf(val, "val%d", i);
        ret += kvlib_set(key, val);
    }

    if (ret != 0)   printf("\n\n\n********************* [Write Performance] FAILED writes\n\n\n");


    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    timeW25 = time_in_micros_new - time_in_micros_old;

    return ret;
}

int write_to_flash_50(void) {
	int ret, i;
    struct timeval tv;

    /* Write to the flash */
    printf("\n\n\n [Write Performance] Writing to Flash till 50%% capacity:\n\n\n");
    ret = 0;

    gettimeofday(&tv,NULL);
    uint64_t time_in_micros_old = 1000000 * tv.tv_sec + tv.tv_usec;
    uint64_t time_in_micros_new = 0;

	for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS / 2) + 1 ; ++i) {
		char key[128], val[128];
		sprintf(key, "key%d", i);
		sprintf(val, "val%d", i);
		ret += kvlib_set(key, val);
	}

    if (ret != 0)   printf("\n\n\n********************* [Write Performance] FAILED writes\n\n\n");


    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    timeW50 = time_in_micros_new - time_in_micros_old;

    return ret;
}

int write_to_flash_75(void) {
    int ret, i;
    struct timeval tv;

    /* Write to the flash */
    printf("\n\n\n [Write Performance] Writing to Flash till 75%% capacity:\n\n\n");
    ret = 0;

    gettimeofday(&tv,NULL);
    uint64_t time_in_micros_old = 1000000 * tv.tv_sec + tv.tv_usec;
    uint64_t time_in_micros_new = 0;

    for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS * 3 / 4) + 1 ; ++i) {
        char key[128], val[128];
        sprintf(key, "key%d", i);
        sprintf(val, "val%d", i);
        ret += kvlib_set(key, val);
    }

    if (ret != 0)   printf("\n\n\n********************* [Write Performance] FAILED writes\n\n\n");


    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    timeW75 = time_in_micros_new - time_in_micros_old;

    return ret;
}

int write_to_flash_100(void) {
    int ret, i;
    struct timeval tv;

    /* Write to the flash */
    printf("\n\n\n [Write Performance] Writing to Flash till 100%% capacity:\n\n\n");
    ret = 0;

    gettimeofday(&tv,NULL);
    uint64_t time_in_micros_old = 1000000 * tv.tv_sec + tv.tv_usec;
    uint64_t time_in_micros_new = 0;

    for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS) + 1 ; ++i) {

        printf("Writing %d / %d\n", i, (PAGES_PER_BLOCK * NUM_BLOCKS));
        char key[128], val[128];
        sprintf(key, "key%d", i);
        sprintf(val, "val%d", i);
        ret += kvlib_set(key, val);
    }

    if (ret != 0)   printf("\n\n\n********************* [Write Performance] FAILED writes\n\n\n");


    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    timeW100 = time_in_micros_new - time_in_micros_old;

    return ret;
}



int update_flash_25(void) {
    int ret, i;
    struct timeval tv;

    /* Write to the flash */
    printf("\n\n\n [Write Performance] Updating Flash till 25%% capacity:\n\n\n");
    ret = 0;

    gettimeofday(&tv,NULL);
    uint64_t time_in_micros_old = 1000000 * tv.tv_sec + tv.tv_usec;
    uint64_t time_in_micros_new = 0;

    for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS / 4) + 1 ; ++i) {
        char key[128], val[128];
        sprintf(key, "key%d", i);
        sprintf(val, "val%d", i);
        ret += kvlib_set(key, val);
    }

    if (ret != 0)   printf("\n\n\n********************* [Write Performance] FAILED updates\n\n\n");


    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    timeU25 = time_in_micros_new - time_in_micros_old;

    return ret;
}

int update_flash_50(void) {
    int ret, i;
    struct timeval tv;

    /* Write to the flash */
    printf("\n\n\n [Write Performance] Updating Flash till 50%% capacity:\n\n\n");
    ret = 0;

    gettimeofday(&tv,NULL);
    uint64_t time_in_micros_old = 1000000 * tv.tv_sec + tv.tv_usec;
    uint64_t time_in_micros_new = 0;

    for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS / 2) + 1 ; ++i) {
        char key[128], val[128];
        sprintf(key, "key%d", i);
        sprintf(val, "val%d", i);
        ret += kvlib_set(key, val);
    }

    if (ret != 0)   printf("\n\n\n********************* [Write Performance] FAILED updates\n\n\n");


    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    timeU50 = time_in_micros_new - time_in_micros_old;

    return ret;
}

int update_flash_75(void) {
    int ret, i;
    struct timeval tv;

    /* Write to the flash */
    printf("\n\n\n [Write Performance] Updating Flash till 75%% capacity:\n\n\n");
    ret = 0;

    gettimeofday(&tv,NULL);
    uint64_t time_in_micros_old = 1000000 * tv.tv_sec + tv.tv_usec;
    uint64_t time_in_micros_new = 0;

    for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS * 3 / 4) + 1 ; ++i) {
        char key[128], val[128];
        sprintf(key, "key%d", i);
        sprintf(val, "val%d", i);
        ret += kvlib_set(key, val);
    }

    if (ret != 0)   printf("\n\n\n********************* [Write Performance] FAILED updates\n\n\n");


    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    timeU75 = time_in_micros_new - time_in_micros_old;

    return ret;
}

int update_flash_100(void) {
    int ret, i;
    struct timeval tv;

    /* Write to the flash */
    printf("\n\n\n [Write Performance] Updating Flash till 100%% capacity:\n\n\n");
    ret = 0;

    gettimeofday(&tv,NULL);
    uint64_t time_in_micros_old = 1000000 * tv.tv_sec + tv.tv_usec;
    uint64_t time_in_micros_new = 0;

    for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS) + 1 ; ++i) {
        char key[128], val[128];
        sprintf(key, "key%d", i);
        sprintf(val, "val%d", i);
        // kvlib_del(key);
        ret += kvlib_set(key, val);
    }

    if (ret != 0)   printf("\n\n\n********************* [Write Performance] FAILED updates\n\n\n");


    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    timeU100 = time_in_micros_new - time_in_micros_old;

    return ret;
}


int main(int argc, char *argv[]) {

    kvlib_format();
    sleep(4);
    write_to_flash_25();
    sleep(4);
    update_flash_25();

    kvlib_format();
    sleep(4);
    write_to_flash_50();
    sleep(4);
    update_flash_50();

    kvlib_format();
    sleep(4);
    write_to_flash_75();
    sleep(4);
    update_flash_75();

    kvlib_format();
    sleep(4);
    write_to_flash_100();
    sleep(4);
    update_flash_100();


    
    printf ("[Write Performance] Time taken to write 25%% of the disk = %luus\n", timeW25);
    printf ("[Write Performance] Time taken to update 25%% of the disk = %luus\n", timeU25);
    printf ("[Write Performance] Time taken to write 50%% of the disk = %luus\n", timeW50);
    printf ("[Write Performance] Time taken to update 50%% of the disk = %luus\n", timeU50);
    printf ("[Write Performance] Time taken to write 75%% of the disk = %luus\n", timeW75);
    printf ("[Write Performance] Time taken to update 75%% of the disk = %luus\n", timeU75);
    printf ("[Write Performance] Time taken to write 100%% of the disk = %luus\n", timeW100);
    printf ("[Write Performance] Time taken to update 100%% of the disk = %luus\n", timeU100);




    return 0;
}
