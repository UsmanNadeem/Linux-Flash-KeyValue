/**
 * Test program using the library to access the storage system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Library header */
#include "kvlib.h"

#define PRINT_PREF " [PERSISTENCE_TEST] "

int write_to_flash(void) {
	int ret, i;

	/* Write to the flash */
	printf(PRINT_PREF " Writing to Flash (Keys 1->300):\n");
	ret = 0;
	for (i = 1; i < 300; i++) {
		char key[128], val[128];
		sprintf(key, "key%d", i);
		sprintf(val, "val%d", i);
		ret += kvlib_set(key, val);
	}
    if (ret != 0) {
        printf(PRINT_PREF "Errors occurred during writes\n");
    }
    return ret;
}

int read_from_flash(void)
{
	int i, ret = 0;
	char buffer[128];
	buffer[0] = '\0';

    /* "get" operation test */
	printf(PRINT_PREF " Reading keys 1 to 300 from Flash\n");

	for (i = 1; i < 300; i++) {
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
        printf(PRINT_PREF "FAILED: Get\n");
    else
        printf(PRINT_PREF "Passed GET\n");

	return 0;

}

int main(int argc, char *argv[]) {
    int i;    
    if ( argc != 2) {
        printf("ERROR: Invalid number of arguments\n");        
        return -1;
    }

    i = atoi(argv[1]);
    
    if (i == 0) {
        printf(PRINT_PREF "Formatting the flash now\n");
        kvlib_format();
        //printf(PRINT_PREF " HERE\n"); 
        write_to_flash();
        read_from_flash();
    } else if (i == 1){
        read_from_flash();

    }
    return 0;
}
