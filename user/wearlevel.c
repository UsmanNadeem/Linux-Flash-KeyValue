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

#define NUM_BLOCKS 9
#define PAGES_PER_BLOCK 64

int write_to_flash_80(void) {
	int ret, i;

    /* Write to the flash */
    printf("\n\n\n [Wear Level Test] Writing to Flash till 80%% capacity:\n\n\n");
    ret = 0;

	for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS * 4 / 5) + 1 ; ++i) {
		char key[128], val[128];
		sprintf(key, "key%d", i);
		sprintf(val, "val%d", i);
		ret += kvlib_set(key, val);
	}

    if (ret != 0)   printf("\n\n\n********************* [Write Performance] FAILED writes\n\n\n");


    return ret;
}


void update_flash_randomly(int numIter)
{
    srand(time(NULL));

    for (int j = 0; j < numIter; ++j) {
        int i;

        printf("[RANDOM Update] Iter: %d/%d\n", j+1, numIter);

        for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS * 4 / 5) + 1; i++) {
            int randNum = rand() % (PAGES_PER_BLOCK * NUM_BLOCKS * 4 / 5) + 1;
            char key[128], val[128];
            sprintf(key, "key%d", randNum);
            sprintf(val, "val%d", randNum);
            kvlib_set(key, val);

        }

    }

}


int main(int argc, char *argv[]) {

    int numIter = 100;


    kvlib_format();
    sleep(2);

    write_to_flash_80();    
    sleep(2);

    update_flash_randomly(numIter);


    return 0;
}
