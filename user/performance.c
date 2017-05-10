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
uint64_t timeW100 = 0;
uint64_t timeRSeq = 0;
uint64_t timeRRand = 0;

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

    if (ret != 0)   printf("\n\n\n********************* [Write Performance] FAILED writes\n");


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

    if (ret != 0)   printf("\n\n\n********************* [Write Performance] FAILED writes\n");


    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    timeW50 += time_in_micros_new - time_in_micros_old;

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
        char key[128], val[128];
        sprintf(key, "key%d", i);
        sprintf(val, "val%d", i);
        ret += kvlib_set(key, val);
    }

    if (ret != 0)   printf("\n\n\n********************* [Write Performance] FAILED writes\n");


    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    timeW100 += time_in_micros_new - time_in_micros_old;

    return ret;
}

int read_from_flash_sequentially(void)
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
	for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS) + 1; i++) {
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
        printf("\n\n\n********************* [SEQ Read Performance] FAILED: Get\n");


    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    timeRSeq += time_in_micros_new - time_in_micros_old;


	return ret;
}

int contains (int* array, int size, int num) {

    for (int i = 0; i < size; ++i) {
        if (array[i] == num) {
            return 1;
        }
    }
    return 0;
}
void fill_array(int* array) {

    srand(time(NULL));
    // srand(41241241);

    for (int i = 0; i < (PAGES_PER_BLOCK * NUM_BLOCKS); i++) {
        int randNum = rand() % (PAGES_PER_BLOCK * NUM_BLOCKS) + 1;

        while (contains(array, i, randNum)) {
            randNum = rand() % (PAGES_PER_BLOCK * NUM_BLOCKS) + 1;
        }

        array[i] = randNum;
    }
}

int read_from_flash_randomly(void)
{
    int i, ret = 0;
    char buffer[128];
    buffer[0] = '\0';
    struct timeval tv;
    int *array = malloc(sizeof(int) * (PAGES_PER_BLOCK * NUM_BLOCKS));

    fill_array(array);

    gettimeofday(&tv,NULL);
    uint64_t time_in_micros_old = 1000000 * tv.tv_sec + tv.tv_usec;
    uint64_t time_in_micros_new = 0;

    /* "get" operation test */
    printf("[RANDOM Read Performance] Reading from a full flash randomly:\n");
    for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS) + 1; i++) {
        char key[128], val[128];
        sprintf(key, "key%d", array[i-1]);
        sprintf(val, "val%d", array[i-1]);
        kvlib_get(key, buffer);
        //printf(PRINT_PREF " Read the Key: %s, val: %s\n", key, buffer);
        if (strcmp(val, buffer)) {
            ret++;
            //printf(PRINT_PREF "Failed GET: \n%s:%s\ngot \"%s\"\n", key, val, buffer);
        }
    }
    if (ret != 0)
        printf("\n\n\n********************* [SEQ Read Performance] FAILED: Get\n");

    gettimeofday(&tv,NULL);
    time_in_micros_new = 1000000 * tv.tv_sec + tv.tv_usec;
    timeRRand += time_in_micros_new - time_in_micros_old;

    free (array);

    return ret;
}


int main(int argc, char *argv[]) {

    int numIter = 25;
for (int i = 0; i < numIter; ++i)
{
    printf("\nIter: %d/%d\n", i+1, numIter);
    
    kvlib_format();

    sleep(2);
    write_to_flash_25();

    kvlib_format();

    sleep(2);
    write_to_flash_50();

    kvlib_format();

    sleep(2);
    write_to_flash_100();

    sleep(2);
    read_from_flash_sequentially();

    sleep(2);
    read_from_flash_randomly();
}
    
    printf ("[Write Performance] Time taken to write 25%% of the disk = %luus\n", timeW25/numIter);
    printf ("[Write Performance] Time taken to write 50%% of the disk = %luus\n", timeW50/numIter);
    printf ("[Write Performance] Time taken to write 100%% of the disk = %luus\n", timeW100/numIter);
    printf ("[SEQ Read Performance] Time taken to read sequentially = %luus\n", timeRSeq/numIter);
    printf ("[RANDOM Read Performance] Time taken to read randomly = %luus\n", timeRRand/numIter);


    return 0;
}
