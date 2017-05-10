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


int write_to_flash_100(void) {
    int ret, i;

    /* Write to the flash */
    printf("\n\n\n Writing to Flash till 100%% capacity:\n\n\n");
    ret = 0;


    for (i = 1; i < (PAGES_PER_BLOCK * NUM_BLOCKS) + 1 ; ++i) {

        // printf("Writing %d / %d\n", i, (PAGES_PER_BLOCK * NUM_BLOCKS));
        char key[128], val[128];
        sprintf(key, "key%d", i);
        sprintf(val, "val%d", i);
        ret += kvlib_set(key, val);
    }

    if (ret != 0)   printf("\n\n\n********************* FAILED writes\n\n\n");


    return ret;
}


int main(int argc, char *argv[]) {

    write_to_flash_100();

    return 0;
}
