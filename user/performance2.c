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

uint64_t timeW25 = 0;
uint64_t timeW50 = 0;
uint64_t timeW75 = 0;
uint64_t timeW100 = 0;
uint64_t timeU25 = 0;
uint64_t timeU50 = 0;
uint64_t timeU75 = 0;
uint64_t timeU100 = 0;
uint64_t timeRead25 = 0;
uint64_t timeRead50 = 0;
uint64_t timeRead75 = 0;
uint64_t timeRead100 = 0;
uint64_t del25 = 0;
uint64_t del50 = 0;
uint64_t del75 = 0;
uint64_t del100 = 0;

int write_to_flash_25(void) {
    int ret, i;
    struct timeval tv;

    /* Write to the flash */
    printf("[Write Performance] Writing to Flash till 25%% capacity:\n");
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
    timeW25 += time_in_micros_new - time_in_micros_old;

    return ret;
}



int write_to_flash_50(void) {
	int ret, i;
    struct timeval tv;

    /* Write to the flash */
    printf("[Write Performance] Writing to Flash till 50%% capacity:\n");
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
    timeW50 += time_in_micros_new - time_in_micros_old;

    return ret;
}

int write_to_flash_75(void) {
    int ret, i;
    struct timeval tv;

    /* Write to the flash */
    printf("[Write Performance] Writing to Flash till 75%% capacity:\n");
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
    timeW75 += time_in_micros_new - time_in_micros_old;

    return ret;
}

int write_to_flash_100(void) {
    int ret, i;
    struct timeval tv;

    /* Write to the flash */
    printf("[Write Performance] Writing to Flash till 100%% capacity:\n");
    ret = 0;

    gettimeofday(&tv,NULL);
    uint64_t time_in_micros_old = 1000000 * tv.tv_sec + tv.tv_usec;
    uint64_t time_in_micros_new = 0;

    for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS) + 1 ; ++i) {

        // printf("Writing %d / %d\n", i, (PAGES_PER_BLOCK * NUM_BLOCKS));
        char key[128], val[128];
        sprintf(key, "key%d", i);
        sprintf(val, "val%d", i);
        ret += kvlib_set(key, val);
    }

    if (ret != 0)   printf("\n\n\n********************* [Write Performance] FAILED writes\n\n\n");


    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    timeW100 += time_in_micros_new - time_in_micros_old;

    return ret;
}


int read_from_flash_sequentially_25(void)
{
    int i, ret = 0;
    char buffer[128];
    buffer[0] = '\0';
    struct timeval tv;


    gettimeofday(&tv,NULL);
    uint64_t time_in_micros_old = 1000000 * tv.tv_sec + tv.tv_usec;
    uint64_t time_in_micros_new = 0;

    /* "get" operation test */
    printf("[SEQ Read Performance] Reading from a 25%% flash sequentially:\n");
    for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS / 4) + 1 ; ++i) {
        char key[128], val[128];
        sprintf(key, "key%d", i);
        sprintf(val, "val%d", i);
        kvlib_get(key, buffer);
        //printf(PRINT_PREF " Read the Key: %s, val: %s\n", key, buffer);
        if (strcmp(val, buffer)) {
            ret++;
            //printf(PRINT_PREF "Failed GET: \n%s:%s\ngot \"%s\"\n", key, val, buffer);
        }
    }
    if (ret != 0)
        printf("\n\n\n********************* [SEQ Read Performance] FAILED: Get\n\n\n");


    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    timeRead25 += time_in_micros_new - time_in_micros_old;

    return ret;
}

int read_from_flash_sequentially_50(void)
{
    int i, ret = 0;
    char buffer[128];
    buffer[0] = '\0';
    struct timeval tv;


    gettimeofday(&tv,NULL);
    uint64_t time_in_micros_old = 1000000 * tv.tv_sec + tv.tv_usec;
    uint64_t time_in_micros_new = 0;

    /* "get" operation test */
    printf("[SEQ Read Performance] Reading from a 50%% flash sequentially:\n");
    for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS / 2) + 1 ; ++i) {
        char key[128], val[128];
        sprintf(key, "key%d", i);
        sprintf(val, "val%d", i);
        kvlib_get(key, buffer);
        //printf(PRINT_PREF " Read the Key: %s, val: %s\n", key, buffer);
        if (strcmp(val, buffer)) {
            ret++;
            //printf(PRINT_PREF "Failed GET: \n%s:%s\ngot \"%s\"\n", key, val, buffer);
        }
    }
    if (ret != 0)
        printf("\n\n\n********************* [SEQ Read Performance] FAILED: Get\n\n\n");


    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    timeRead50 += time_in_micros_new - time_in_micros_old;

    return ret;
}

int read_from_flash_sequentially_75(void)
{
    int i, ret = 0;
    char buffer[128];
    buffer[0] = '\0';
    struct timeval tv;


    gettimeofday(&tv,NULL);
    uint64_t time_in_micros_old = 1000000 * tv.tv_sec + tv.tv_usec;
    uint64_t time_in_micros_new = 0;

    /* "get" operation test */
    printf("[SEQ Read Performance] Reading from a 75%% flash sequentially:\n");
    for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS*3 / 4) + 1 ; ++i) {
        char key[128], val[128];
        sprintf(key, "key%d", i);
        sprintf(val, "val%d", i);
        kvlib_get(key, buffer);
        //printf(PRINT_PREF " Read the Key: %s, val: %s\n", key, buffer);
        if (strcmp(val, buffer)) {
            ret++;
            //printf(PRINT_PREF "Failed GET: \n%s:%s\ngot \"%s\"\n", key, val, buffer);
        }
    }
    if (ret != 0)
        printf("\n\n\n********************* [SEQ Read Performance] FAILED: Get\n\n\n");


    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    timeRead75 += time_in_micros_new - time_in_micros_old;

    return ret;
}

int read_from_flash_sequentially_100(void)
{
    int i, ret = 0;
    char buffer[128];
    buffer[0] = '\0';
    struct timeval tv;


    gettimeofday(&tv,NULL);
    uint64_t time_in_micros_old = 1000000 * tv.tv_sec + tv.tv_usec;
    uint64_t time_in_micros_new = 0;

    /* "get" operation test */
    printf("[SEQ Read Performance] Reading from a full flash sequentially:\n");
    for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS) + 1 ; ++i) {
        char key[128], val[128];
        sprintf(key, "key%d", i);
        sprintf(val, "val%d", i);
        kvlib_get(key, buffer);
        //printf(PRINT_PREF " Read the Key: %s, val: %s\n", key, buffer);
        if (strcmp(val, buffer)) {
            ret++;
            //printf(PRINT_PREF "Failed GET: \n%s:%s\ngot \"%s\"\n", key, val, buffer);
        }
    }
    if (ret != 0)
        printf("\n\n\n********************* [SEQ Read Performance] FAILED: Get\n\n\n");


    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    timeRead100 += time_in_micros_new - time_in_micros_old;

    return ret;
}
int update_flash_25(void) {
    int ret, i;
    struct timeval tv;

    /* Write to the flash */
    printf("[Write Performance] Updating Flash till 25%% capacity:\n");
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
    timeU25 += time_in_micros_new - time_in_micros_old;

    return ret;
}

int update_flash_50(void) {
    int ret, i;
    struct timeval tv;

    /* Write to the flash */
    printf("[Write Performance] Updating Flash till 50%% capacity:\n");
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
    timeU50 += time_in_micros_new - time_in_micros_old;

    return ret;
}

int update_flash_75(void) {
    int ret, i;
    struct timeval tv;

    /* Write to the flash */
    printf("[Write Performance] Updating Flash till 75%% capacity:\n");
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
    timeU75 += time_in_micros_new - time_in_micros_old;

    return ret;
}

int update_flash_100(void) {
    int ret, i;
    struct timeval tv;

    /* Write to the flash */
    printf("[Write Performance] Updating Flash till 100%% capacity:\n");
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

    if (ret != 0)   printf("\n\n\n********************* [Write Performance] FAILED updates\n");


    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    timeU100 += time_in_micros_new - time_in_micros_old;

    return ret;
}

int delete_flash_25(void)
{
    int i, ret = 0;
    struct timeval tv;


    gettimeofday(&tv,NULL);
    uint64_t time_in_micros_old = 1000000 * tv.tv_sec + tv.tv_usec;
    uint64_t time_in_micros_new = 0;

    printf("[SEQ Delete Performance] Deleting from a 25%% full flash sequentially:\n");
    for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS/4) + 1 ; ++i) {
        char key[128];
        sprintf(key, "key%d", i);
        kvlib_del(key);
    }

    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    del25 += time_in_micros_new - time_in_micros_old;

    return ret;
}

int delete_flash_50(void)
{
    int i, ret = 0;
    struct timeval tv;


    gettimeofday(&tv,NULL);
    uint64_t time_in_micros_old = 1000000 * tv.tv_sec + tv.tv_usec;
    uint64_t time_in_micros_new = 0;

    printf("[SEQ Delete Performance] Deleting from a 50%% full flash sequentially:\n");
    for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS/2) + 1 ; ++i) {
        char key[128];
        sprintf(key, "key%d", i);
        kvlib_del(key);
    }

    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    del50 += time_in_micros_new - time_in_micros_old;

    return ret;
}

int delete_flash_75(void)
{
    int i, ret = 0;
    struct timeval tv;


    gettimeofday(&tv,NULL);
    uint64_t time_in_micros_old = 1000000 * tv.tv_sec + tv.tv_usec;
    uint64_t time_in_micros_new = 0;

    printf("[SEQ Delete Performance] Deleting from a 75%% full flash sequentially:\n");
    for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS*3/4) + 1 ; ++i) {
        char key[128];
        sprintf(key, "key%d", i);
        kvlib_del(key);
    }

    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    del75 += time_in_micros_new - time_in_micros_old;

    return ret;
}

int delete_flash_100(void)
{
    int i, ret = 0;
    struct timeval tv;


    gettimeofday(&tv,NULL);
    uint64_t time_in_micros_old = 1000000 * tv.tv_sec + tv.tv_usec;
    uint64_t time_in_micros_new = 0;

    printf("[SEQ Delete Performance] Deleting from a 100%% full flash sequentially:\n");
    for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS) + 1 ; ++i) {
        char key[128];
        sprintf(key, "key%d", i);
        kvlib_del(key);
    }

    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    del100 += time_in_micros_new - time_in_micros_old;

    return ret;
}

int main(int argc, char *argv[]) {

    int numIter = 11;
for (int i = 0; i < numIter; ++i)
{
    printf("\nIter: %d/%d\n", i+1, numIter);

    kvlib_format();
    sleep(2);
    write_to_flash_25();
    sleep(2);
    read_from_flash_sequentially_25();
    update_flash_25();
    sleep(2);
    delete_flash_25();

    kvlib_format();
    sleep(2);
    write_to_flash_50();
    read_from_flash_sequentially_50();
    sleep(2);
    update_flash_50();
    sleep(2);
    delete_flash_50();

    kvlib_format();
    sleep(2);
    write_to_flash_75();
    read_from_flash_sequentially_75();
    sleep(2);
    update_flash_75();
    sleep(2);
    delete_flash_75();

    kvlib_format();
    sleep(2);
    write_to_flash_100();
    read_from_flash_sequentially_100();
    sleep(2);
    update_flash_100();
    sleep(2);
    delete_flash_100();
}


    
    printf ("[Performance] Time taken to write\t25%% of the disk = %luus\n", timeW25/numIter);
    printf ("[Performance] Time taken to update\t25%% of the disk = %luus\n", timeU25/numIter);
    printf ("[Performance] Time taken to read\t25%% of the disk = %luus\n", timeRead25/numIter);
    printf ("[Performance] Time taken to delete\t25%% of the disk = %luus\n", del25/numIter);
printf("\n");

    printf ("[Performance] Time taken to write\t50%% of the disk = %luus\n", timeW50/numIter);
    printf ("[Performance] Time taken to update\t50%% of the disk = %luus\n", timeU50/numIter);
    printf ("[Performance] Time taken to read\t50%% of the disk = %luus\n", timeRead50/numIter);
    printf ("[Performance] Time taken to delete\t50%% of the disk = %luus\n", del50/numIter);
printf("\n");

    printf ("[Performance] Time taken to write\t75%% of the disk = %luus\n", timeW75/numIter);
    printf ("[Performance] Time taken to update\t75%% of the disk = %luus\n", timeU75/numIter);
    printf ("[Performance] Time taken to read\t75%% of the disk = %luus\n", timeRead75/numIter);
    printf ("[Performance] Time taken to delete\t75%% of the disk = %luus\n", del75/numIter);
printf("\n");

    printf ("[Performance] Time taken to write\t100%% of the disk = %luus\n", timeW100/numIter);
    printf ("[Performance] Time taken to update\t100%% of the disk = %luus\n", timeU100/numIter);
    printf ("[Performance] Time taken to read\t100%% of the disk = %luus\n", timeRead100/numIter);
    printf ("[Performance] Time taken to delete\t100%% of the disk = %luus\n", del100/numIter);
printf("\n");



    return 0;
}
