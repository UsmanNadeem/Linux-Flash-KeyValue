/**
 * This file contains the prototype core functionalities.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mtd/mtd.h>
#include <linux/vmalloc.h>
#include <linux/string.h>

#include "core.h"
#include "device.h"

#define PRINT_PREF KERN_INFO "[LKP_KV]: "
#define GC_PREFIX PRINT_PREF "GARBAGE_COLLECTOR: "

/* prototypes */
int init_config(int mtd_index);
void destroy_config(void);
void print_config(void);
int write_page(int page_index, const char *buf);
int read_page(int page_index, char *buf);
void format_callback(struct erase_info *e);
int get_next_page_index_to_write(void);
int get_next_free_block(void);
int init_scan(void);
int delete(directory_entry* a);
directory_entry *key_exists(const char *key);


void writeFSMetadata(void);
int initialize_firsttime(int mtd_index);
int readFSMetadata(int mtd_index);
int read_page_mtd(int page_index, char *buf, struct mtd_info *mtd);
int format(void);
int eraseBlock(int blockNum, uint64_t number);
int garbage_collect(void);
int select_block_for_gc(int *, int, int);
int get_num_free_blocks(void);
void print_flash_state(void);
void read_key_val_from_page(char *, char *, char *);
int move_data(int, int, int, char *);


lkp_kv_cfg config;

/* The module takes one parameter which is the index of the target flash
 * partition */
int MTD_INDEX = -1;
module_param(MTD_INDEX, int, 0);
MODULE_PARM_DESC(MTD_INDEX, "Index of target mtd partition");

/**
 * Module initialization function
 */
static int __init lkp_kv_init(void)
{
	printk(PRINT_PREF "Loading... \n");

	if (init_config(MTD_INDEX) != 0) {
		printk(PRINT_PREF "Initialization error\n");
		return -1;
	}

	if (device_init() != 0) {
		printk(PRINT_PREF "Virtual device creation error\n");
		return -1;
	}

	return 0;
}

/**
 * Module exit function
 */
static void __exit lkp_kv_exit(void)
{
	printk(PRINT_PREF "Exiting ... \n");
	device_exit();
	destroy_config();
}

/**
 * Global state initialization, return 0 when ok, -1 on error
 */
int init_config(int mtd_index)
{
	if (mtd_index == -1) {
		printk(PRINT_PREF
		       "Error, flash partition index missing, should be"
		       " indicated for example like this: MTD_INDEX=5\n");
		return -1;
	}


	printk(PRINT_PREF "Trying to read existing metadata\n");
	if (readFSMetadata(mtd_index) == -1) {
		printk(PRINT_PREF "Init metadata read error.\n\n*************Creating new metadata and formating.\n");
		return initialize_firsttime(mtd_index);
	}
	printk(PRINT_PREF "*************Using old metadata.\n");




	return 0;
}


int readFSMetadata(int mtd_index) {

	/* The flash partition is manipulated by caling the driver, through the
	 * mtd_info object. There is one of these object per flash partition */
    
    printk("************* read_fs_metadata *************\n");	
    
    struct mtd_info *mtd = get_mtd_device(NULL, mtd_index);

	if (mtd == NULL)
		return -1;

	int block_size = mtd->erasesize;
	int page_size = mtd->writesize;
	int pages_per_block = block_size / page_size;
    printk("pages per block calculated\n");
	uint64_t tmp_blk_num = mtd->size;
	printk("dodiv\n");
    do_div(tmp_blk_num, (uint64_t) mtd->erasesize);
    printk("dodiv done\n");
	int nb_blocks = (int)tmp_blk_num;


    printk("*************** Read configuration ****************** \n");

	char *buffer;

	uint64_t numPages = 0;
	uint64_t j = 0;
	int i = 0;
	int k = 0;

	uint64_t size = 0;
	size+= sizeof(lkp_kv_cfg);
	size+= sizeof(blk_info)* nb_blocks;
	size+= sizeof(page_state) * nb_blocks * 
		pages_per_block;

	int totalPages = nb_blocks * pages_per_block;
	for (j = 0; j < size;) {
		j += page_size;
		numPages += 1;
	}

    printk("Before config.pagesize\n");

	buffer = (char *)vmalloc(page_size * numPages);
	for (i = 0; i < page_size * numPages; i++)
		buffer[i] = 0x0;

    // printk("*************** Going to read page ****************** \n");


    /* read page */
	printk(PRINT_PREF "Read starting from page_index: %d into buffer:%p\n", 0, (void*)buffer);
	for (i = 0; i < numPages; ++i) {
		uint64_t address = ((uint64_t) i) * ((uint64_t) page_size);

		int err = read_page_mtd(i, buffer + address, mtd);
	    if (err != 0) {
		    /*
859          * In the absence of an error, drivers return a non-negative integer
860          * representing the maximum number of bitflips that were corrected on
861          * any one ecc region (if applicable; zero otherwise).
862          */
			printk(PRINT_PREF "Error: %d in readFSMetadata->read_page_mtd\n", err);
			printk(PRINT_PREF "Error reading pagesize:%d page_index: %d into buffer:%p@0x%x\n", mtd->writesize, i, 
				(void*)buffer, (unsigned int) address);
			vfree(buffer);
			return -1;
		}
	}


    // printk("*************** Page read complete ****************** \n");


	memcpy(&config, buffer, sizeof(lkp_kv_cfg));


	if (config.mtd_index != mtd_index)
		return -1;

	config.mtd = get_mtd_device(NULL, mtd_index);

	if (config.mtd == NULL || config.nb_blocks != nb_blocks || config.block_size != block_size)
		return -1;

	config.blocks = (blk_info *) vmalloc((config.nb_blocks) * sizeof(blk_info));


    // printk("*************** Iterating over blocks ****************** \n");



	for (i = 0; i < config.nb_blocks; ++i) {
		
		memcpy(&(config.blocks[i]), 
			buffer + sizeof(lkp_kv_cfg) + (i*sizeof(blk_info)) + (i*config.pages_per_block*sizeof(page_state)) 
			, sizeof(blk_info));
		
			config.blocks[i].pages_states = (page_state *) vmalloc(config.pages_per_block * sizeof(page_state));

		for (k = 0; k < config.pages_per_block; ++k) {
			
			memcpy(&(config.blocks[i].pages_states[k]), 
				buffer + sizeof(lkp_kv_cfg) + ((i+1)*sizeof(blk_info)) + (i*config.pages_per_block*sizeof(page_state))
				+ (k*sizeof(page_state))
				, sizeof(page_state));
		
		}
	}
	vfree(buffer);

    // printk("*************** freed buffer ****************** \n");



	int dirInOnePage = page_size/sizeof(directory_entry);
	int numDirPages = 0;
	uint64_t remainingPages = totalPages - numPages;

	while ((numDirPages*dirInOnePage) < remainingPages) {
		--remainingPages;
		++numDirPages;
	}
	uint64_t MAX_KEYS = remainingPages;


	size = numDirPages * config.page_size;

	tmp_blk_num = numPages;
	numPages = 0;
	for (j = 0; j < size;) {
		j += config.page_size;
		numPages += 1;
	}

	buffer = (char *)vmalloc(config.page_size * numPages);
	for (i = 0; i < config.page_size * numPages; i++)
		buffer[i] = 0x0;

    // printk("*************** Reading page again ****************** \n");


    /* read page */
	printk(PRINT_PREF "Read starting from page_index: %llu into buffer:%p\n", tmp_blk_num, (void*)(buffer));
	for (i = 0; i < numPages; ++i) {
		// int read_page(int page_index, char *buf)
		// 
		// uint64_t address = ((uint64_t) tmp_blk_num+i) * ((uint64_t) config.page_size);
		uint64_t address = ((uint64_t) tmp_blk_num+i);
		uint64_t baseOffset = ((uint64_t) i) * ((uint64_t) config.page_size);  // offset in the buffer
		int err = read_page(address, buffer + baseOffset);
	    if (err != 0) {
			printk(PRINT_PREF "Error: %d in readFSMetadata->read_page\n", err);
			printk(PRINT_PREF "Reading from page_index: %llu into buffer@%p\n", address, (void*)(buffer+baseOffset));
			vfree(buffer);
			return -1;
		}
	}

    // printk("*************** Page read complete  ****************** \n");



	config.dir.list = (directory_entry *) vmalloc((MAX_KEYS) * sizeof(directory_entry));

	for (i = 0; i < config.MAX_KEYS; ++i) {

		memcpy(&(config.dir.list[i])
			, buffer + sizeof(directory_entry)*i
			, sizeof(directory_entry));
	}
	vfree(buffer);

	/* Semaphore initialized to 1 (available) */
	sema_init(&config.format_lock, 1);

	print_config();

	return 0;
}

/**
 * Launch time metadata creation: flash is scanned to determine which flash 
 * blocs and pages are free/occupied. Return 0 when ok, -1 on error
 */
int initialize_firsttime(int mtd_index)
{
	int i, j;

	uint64_t tmp_blk_num;

	int numPages = 0;
	uint64_t size = 0;

	config.mtd_index = mtd_index;
	config.mtd = get_mtd_device(NULL, mtd_index);

	if (config.mtd == NULL)
		return -1;
	config.format_done = 0;
	config.read_only = 0;
	config.block_size = config.mtd->erasesize;
	config.page_size = config.mtd->writesize;
	config.pages_per_block = config.block_size / config.page_size;

	tmp_blk_num = config.mtd->size;
	do_div(tmp_blk_num, (uint64_t) config.mtd->erasesize);
	config.nb_blocks = (int)tmp_blk_num;

	/* Semaphore initialized to 1 (available) */
	sema_init(&config.format_lock, 1);

	print_config();


	/* metadata initialization */
	config.blocks =
	    (blk_info *) vmalloc((config.nb_blocks) * sizeof(blk_info));
	for (i = 0; i < config.nb_blocks; i++) {
		config.blocks[i].state = BLK_FREE;
		config.blocks[i].wipeCount = 0;
		config.blocks[i].invalid_pages = 0;
		config.blocks[i].free_pages = config.pages_per_block;

		config.blocks[i].pages_states =
		    (page_state *) vmalloc(config.pages_per_block *
					   sizeof(page_state));
		for (j = 0; j < config.pages_per_block; j++)
			config.blocks[i].pages_states[j] = PG_FREE;
	}



	size+= sizeof(lkp_kv_cfg);
	size+= sizeof(blk_info)* config.nb_blocks;
	size+= sizeof(page_state) * config.nb_blocks * 
		config.pages_per_block;

	for (j = 0; j < size;) {
		j += config.page_size;
		numPages += 1;
	}

	int dirInOnePage = config.page_size/sizeof(directory_entry);
	int numDirPages = 0;
	int totalPages = config.nb_blocks * config.pages_per_block;
	uint64_t remainingPages = totalPages - numPages;

	while ((numDirPages*dirInOnePage) < remainingPages) {
		--remainingPages;
		++numDirPages;
	}
	config.MAX_KEYS = remainingPages;
	numPages += numDirPages;

	config.metadata_blocks = 0;
	for (i=0, j = 0; j < numPages;) {
		j += config.pages_per_block;
		i += 1;
	}
	config.metadata_blocks = i;
	config.current_block = config.metadata_blocks;
	config.blocks[config.metadata_blocks].state = BLK_USED;

	config.current_page_offset = 0;

	config.dir.list = (directory_entry *) vmalloc((config.MAX_KEYS) * sizeof(directory_entry));

	for (i = 0; i < config.MAX_KEYS; ++i) {
		config.dir.list[i].keyHash = 0;
		config.dir.list[i].state = KEY_DELETED;
	}
	format();
	writeFSMetadata();
	return 0;
}

/**
 * Freeing stuff on exit
 */
void destroy_config(void)  //todo fix
{
	int i;
	writeFSMetadata();
	put_mtd_device(config.mtd);
	for (i = 0; i < config.nb_blocks; i++)
		vfree(config.blocks[i].pages_states);
	vfree(config.blocks);
}


// hash function taken from 
// Linux/net/irda/irqueue.c
static __u32 hash( const char* name) {
    __u32 h = 0;
    __u32 g;
 
    while(*name) {
        h = (h<<4) + *name++;
        if ((g = (h & 0xf0000000)))
            h ^=g>>24;
            h &=~g;
        }
    return h;
}

/**
 * Adding a key-value couple. Returns -1 when ok and a negative value on error:
 * -1 when the size to write is too big
 * -2 when the key already exists
 * -3 when we are in read-only mode
 * -4 when the MTD driver returns an error
 */
int set_keyval(const char *key, const char *val)
{
	int key_len, val_len, i, ret, old_offset, old_block;
	char *buffer;
	directory_entry* dirToAdd;
	__u32 _hash = hash(key);

	key_len = strlen(key);
	val_len = strlen(val);
	dirToAdd = NULL;

	if ((key_len + val_len + 2 * sizeof(int)) > config.page_size) {
		/* size to write is too big */
		return -1;
	}

	//  Check for a directory entry with the same key
	for (i = 0; i < config.MAX_KEYS; ++i) {
		if (config.dir.list[i].keyHash == _hash) {

			if (config.dir.list[i].state == KEY_DELETED) {
				dirToAdd = &(config.dir.list[i]);
				break;
			} else if (config.dir.list[i].state == KEY_VALID) {
				//printk(PRINT_PREF "Key \"%s\" already exists Updating it\n", key);
				delete(&(config.dir.list[i]));
				dirToAdd = &(config.dir.list[i]);
				break;
			}
		}
	}

	// directory entry not found
	if (dirToAdd == NULL) {
		// choose the first invalid directory entry pointer
		for (i = 0; i < config.MAX_KEYS; ++i) {
			if (config.dir.list[i].state == KEY_DELETED) {
				dirToAdd = &(config.dir.list[i]);
				break;
			}
		}
	}

	// no free space
	if (dirToAdd == NULL) {
		printk(PRINT_PREF
		       "no free block left... switching to read-only mode\n");
		config.read_only = 1;
		return -3;  // readonly mode
	}

	/* the buffer that we are going to write on flash */
	buffer = (char *)vmalloc(config.page_size * sizeof(char));



	/* prepare the buffer we are going to write on flash */
	for (i = 0; i < config.page_size; i++)
		buffer[i] = 0x0;

	/* key size ... */
	memcpy(buffer, &key_len, sizeof(int));
	/* ... value size ... */
	memcpy(buffer + sizeof(int), &val_len, sizeof(int));
	/* ... the key itself ... */
	memcpy(buffer + 2 * sizeof(int), key, key_len);
	/* ... then the value itself. */
	memcpy(buffer + 2 * sizeof(int) + key_len, val, val_len);

	/* actual write on flash */
	//printk("Writing key: %s, Value: %s\n", key, val);
  //  printk("Writing on block: %d, page_offset: %d", config.current_block, config.current_page_offset);
	old_offset = config.current_page_offset;
	old_block = config.current_block;

	config.blocks[old_block].free_pages--;

    ret =
	    write_page(config.current_block * config.pages_per_block +
		       config.current_page_offset, buffer);

	vfree(buffer);

	if (ret == -1)		/* read-only */
		return -3;
	else if (ret == -2)	/* write error */
		return -4;
    
	// successfully written. Update directory metadata.
	dirToAdd->keyHash = _hash;
	dirToAdd->state = KEY_VALID;
	dirToAdd->block = old_block;
	// dirToAdd->page_offset = config.current_page_offset-1;  // write_page increases the offset. could give wrong offset 
														   // if we go to the next page
	dirToAdd->page_offset = old_offset;
	writeFSMetadata();   // todo remove from here and set with timer and lock
	return 0;
}

/**
 * Getting a value from a key.
 * Returns the index of the page containing the key/value couple on success,
 * and a negative number on error:
 * -1 when the key is not found
 * -2 on MTD read error
 */
int get_keyval(const char *key, char *val)
{
	int i;
	char *buffer;
	directory_entry *entry = NULL;
    int page_index;
    int key_len, val_len;
    char *cur_key, *cur_val;
    __u32 _hash = hash(key);    /* Compute hash for the key */

	buffer = (char *)vmalloc(config.page_size * sizeof(char));

	/*  Search for key in dictionary entry */
    for (i = 0; i < config.MAX_KEYS; ++i) {
        if (config.dir.list[i].keyHash == _hash) {
		/* Key found. Check if the entry is valid */
		    if (config.dir.list[i].state == KEY_VALID) {
                //printk(PRINT_PREF "Key \"%s\" found\n", key);
                entry = &config.dir.list[i];
                break;
		    }
		}
    }

	if (entry) {
        page_index = (entry->block * config.pages_per_block) + entry->page_offset;

        /* read page */
        if (read_page(page_index, buffer) != 0) {
			vfree(buffer);
			return -2;
		}

        memcpy(&key_len, buffer, sizeof(int));
        memcpy(&val_len, buffer + sizeof(int), sizeof(int));
        
        /* TODO: These checks are not necessary.*/
		if (key_len != 0xFFFFFFFF) {	/* shoud always be true */
			cur_key = buffer + 2 * sizeof(int);
			cur_val = buffer + 2 * sizeof(int) + key_len;
            if (!strncmp(cur_key, key, strlen(key))) {
				/* key on the page is same as input key. Read value */
				memcpy(val, cur_val, val_len);
			    val[val_len] = '\0';
				vfree(buffer);
				return page_index;
			}
		}
    }

    vfree(buffer);
    return -1;
}

/**
 * Delete a Key-Value pair 
 * Returns 0 on success, -1 if key does not exist.
 */
int del_keyval(const char *key)
{
    int i;
    __u32 _hash = hash(key); 

    for (i = 0; i < config.MAX_KEYS; ++i) {
        if (config.dir.list[i].keyHash == _hash) {
            if (config.dir.list[i].state == KEY_VALID) {
                /* Valid key found */
                return delete(&config.dir.list[i]);
            }  
        }
    }
    return -1;
}

/**
 * Sets metadata info of entry to deleted
 * Returns -1 if invalid entry
 */
int delete(directory_entry *entry)
{
    if (!entry) 
        return -1;
    entry->state = KEY_DELETED;
    config.blocks[entry->block].pages_states[entry->page_offset] = PG_DELETED;
    config.blocks[entry->block].invalid_pages++;
    return 0;
}

/**
 * Update a Key-Value pair
 * -1 when the size to write is too big
 * -2 when the key does not exist
 * -3 when we are in read-only mode
 * -4 when the MTD driver returns an error
 */
int update_keyval(const char *key, const char *val)
{
    int ret = 0;
    directory_entry *entry = key_exists(key);
    if (entry) {
        delete(entry);
        ret = set_keyval(key, val);
        //WARN_ON(ret != 2);
        return ret;
    }
    return -2;
}

/**
 * Checks whether key exists in the metadata.
 * Returns the entry if key exists, otherwise returns NULL
 */
directory_entry *key_exists(const char *key)
{
    int i;
    __u32 _hash = hash(key);
     
    for (i = 0; i < config.MAX_KEYS; ++i)
        if (config.dir.list[i].keyHash == _hash)
            if (config.dir.list[i].state == KEY_VALID) 
                return &(config.dir.list[i]);

    return NULL;
}


/**
 *
 */
void read_key_val_from_page(char *key, char *val, char *buffer) {

    int key_len, val_len;
    char *cur_key, *cur_val;

    memcpy(&key_len, buffer, sizeof(int));
    memcpy(&val_len, buffer + sizeof(int), sizeof(int));
       
    if (key_len != 0xFFFFFFFF) {
        cur_key = buffer + 2 * sizeof(int);
        cur_val = buffer + 2 * sizeof(int) + key_len;
        memcpy(key, cur_key, key_len);
        key[key_len] = '\0';
        memcpy(val, cur_val, val_len);
        val[val_len] = '\0';
    }
//    printk(GC_PREFIX "Read key from page: Key: %s, Val: %s\n", key, val);
    
}

/**
 * Update metadata for a Key in RAM
 */
void update_key_val_metadata(char *key, int new_block, int new_page_offset) {
    __u32 _hash = hash(key);
    int i;
	directory_entry *entry = NULL;
 
    //printk("Updating metadata for Key: %s\n", key);

    /*Search for key in dictionary entry */
    for (i = 0; i < config.MAX_KEYS; ++i) {
        if (config.dir.list[i].keyHash == _hash) {
		/* Key found. Check if the entry is valid */
		    if (config.dir.list[i].state == KEY_VALID) {
                //printk(PRINT_PREF "Key \"%s\" found\n", key);
                entry = &config.dir.list[i];
                break;
		    }
		}
    }
    
    if (entry) {
        /* update metadata here */ 
        entry->block = new_block;
        entry->page_offset = new_page_offset;
    }
}


/**
 *
 */
int select_block_for_gc(int *exclude_blocks, int num_excluded_blocks, int req_pages) {
    int i, j;
    int selected_block_id = -1;
    unsigned int blk_victim_potential = UINT_MAX;
    unsigned int new_blk_victim_potential;
    bool should_exclude = false;

    for (i = config.metadata_blocks; i < config.nb_blocks; ++i) {
        for (j = 0; j < num_excluded_blocks; j++) {
            if (i == exclude_blocks[j]) {
                should_exclude = true;
                break;
            }
        }
        
        if (should_exclude)
            continue;

        if (config.blocks[i].state == BLK_USED) {
 
            if (config.blocks[i].invalid_pages == 0)
                continue;           

            new_blk_victim_potential = (config.blocks[i].wipeCount * 100) / config.blocks[i].invalid_pages;
 
//            printk(GC_PREFIX " Block selection: Block: %d - Ranking: %d Invalid_Pages: %d, Erase_Count: %llu\n",
//                   i, new_blk_victim_potential, config.blocks[i].invalid_pages, config.blocks[i].wipeCount);
           
            if (new_blk_victim_potential <= blk_victim_potential) {
                if (config.pages_per_block - (config.blocks[i].free_pages + 
                            config.blocks[i].invalid_pages) <= req_pages) {
                    selected_block_id = i;
                    blk_victim_potential = new_blk_victim_potential;
                }
            }
        }
        should_exclude = false;
    }
    //printk(GC_PREFIX "Selected Block For GC (select_block_for_gc): %d\n", selected_block_id);
    return selected_block_id;
}


/**
 * Function to perform garbage collection.
 * Victim block selection criteria: block with highest invalid_pages/wipe_count ratio.
 */
int garbage_collect() {
    int i;
    int selected_block_id = 0;
    int selected_block_ids[config.nb_blocks];
    int num_selected_blocks = 0;
    int selected_free_blk_id = -1;
    unsigned int wipe_count = UINT_MAX;
    unsigned int remaining_pages = config.pages_per_block;
    char *blk_data_buf;

    /* Select a free block to move the data */
    for (i = config.metadata_blocks; i < config.nb_blocks; ++i) {
//        printk(GC_PREFIX " Free Block Selection: Block: %d - Block-state: %d - Erase_Count: %llu\n", i, config.blocks[i].state, config.blocks[i].wipeCount);
        if (config.blocks[i].wipeCount <= wipe_count 
                && config.blocks[i].state == BLK_FREE) {
            selected_free_blk_id = i;
            wipe_count = config.blocks[i].wipeCount;
        }
    }

    printk(GC_PREFIX "Selected Free block in GC: %d\n", selected_free_blk_id);


    /* If there was no free block, select a block with minimum number of 
     * valid pages and "garbage collect" it */
    /* TODO: Should consider WipeCount here as well */
    int blk_valid_pages = config.pages_per_block - 
        (config.blocks[config.metadata_blocks].free_pages + 
        config.blocks[config.metadata_blocks].invalid_pages);

    if (selected_free_blk_id == -1) {
        printk(GC_PREFIX "No free block found. Now trying to find block with most garbage\n");
        for (i = config.metadata_blocks; i < config.nb_blocks; ++i) {
            printk(GC_PREFIX "Block: %d - free_pages: %llu - invalid_pages: %d\n", i, config.blocks[i].free_pages, config.blocks[i].invalid_pages);
            if (config.pages_per_block - (config.blocks[i].free_pages + 
                     config.blocks[i].invalid_pages) <= blk_valid_pages) {
                selected_free_blk_id = i;
                blk_valid_pages = config.pages_per_block - 
                    (config.blocks[i].free_pages + config.blocks[i].invalid_pages);
            }
        }
        /* Now, we have a block with minimum # of valid pages which we will  
         * write to. Add this block to selected_block_ids so that its data 
         * can also be selected for garbage collection. We also need to find 
         * other blocks for GC, adjust remaining_pages variable to the free +
         * invalid pages of this block */
        selected_block_ids[0] = selected_free_blk_id;
        num_selected_blocks = 1;
        remaining_pages = config.blocks[selected_free_blk_id].free_pages + 
            config.blocks[selected_free_blk_id].invalid_pages;
    }
    
       
    printk(GC_PREFIX "Selected Free block in GC: %d\n", selected_free_blk_id);

  
    /* Select blocks for garbage collection */

    while (true) {
        selected_block_id = select_block_for_gc(selected_block_ids, num_selected_blocks, remaining_pages);
        if (selected_block_id == -1) {
            //num_selected_blocks += 1;
            break;
        }
        /* keep track of how many pages we can still fill in the selected_free_block */
        remaining_pages -= config.pages_per_block - (config.blocks[selected_block_id].free_pages + 
                config.blocks[selected_block_id].invalid_pages);
        
        selected_block_ids[num_selected_blocks++] = selected_block_id;
    
    }
    if (num_selected_blocks == 0 && selected_free_blk_id != -1) {
        printk(GC_PREFIX "No block found for GC (invalid_pages = 0). Free block = %d\n", selected_free_blk_id);
        config.current_block = selected_free_blk_id;
        config.current_page_offset = 0;
        config.blocks[selected_free_blk_id].state = BLK_USED;
        config.blocks[selected_free_blk_id].free_pages = remaining_pages;
        return 0;
    } 


    /* At this point, num_selected_blocks have been selected for GC and 
     * their IDs are present in selected_block_ids array */
    
    printk(GC_PREFIX "Selected Free block: %d\n", selected_free_blk_id);
    for (i = 0; i < num_selected_blocks; i++) 
        printk(GC_PREFIX "Selected Block for GC: %d\n", selected_block_ids[i]);

    
    blk_data_buf = (char *) vmalloc(config.pages_per_block * config.page_size);
    int page_index = 0, j = 0;
    
    /* Copy all valid data from the blocks selected for GC into the buffer*/
    int buf_offset = 0;
    if (blk_data_buf != NULL) {
        printk(GC_PREFIX " Copying all valid data from GC selected blocks into buffers\n");
        for (i = 0; i < num_selected_blocks; i++) {
            for (j = 0; j < config.pages_per_block; j++) {
                if (config.blocks[selected_block_ids[i]].pages_states[j] == PG_VALID) {
                    page_index = (selected_block_ids[i] * config.pages_per_block) + buf_offset;
                    read_page(page_index, blk_data_buf + (buf_offset * config.page_size));
                    buf_offset++;
                    //memcpy(blk_data_buf + j * config.page_size, page_data_buf, config.page_size);
                    //blk_data_buf_offset += config.page_size;
                }
            }
        }
    }

    /* Write the data into the selected free block */
    int num_pages_to_write = config.pages_per_block - remaining_pages;
    page_index = selected_free_blk_id * config.pages_per_block;
    /* If we could not find any blocks for GC and there's only one block 
     * (selected_free_blk_id), we erase this block and write the data to it
     * again and adjust the current block and page_offset in config.*/
    if(num_selected_blocks == 1 && selected_block_ids[0] == 
            selected_free_blk_id) {
        printk(GC_PREFIX "No free blocks found. 'Garbage Collecting' data from "
                "block with most invalid pages\n");
        /* Erase the block and write to it */
        eraseBlock(selected_free_blk_id, 1);
       
        move_data(selected_free_blk_id, page_index, num_pages_to_write, blk_data_buf);
        return 0;
    }
    

    /* We get to here because free block is different from the block(s)
     * we are going to erase*/
    move_data(selected_free_blk_id, page_index, num_pages_to_write, blk_data_buf);

    /* Erase the GC selected blocks */
    for (i = 0; i < num_selected_blocks; i++) {
        printk(GC_PREFIX "Erasing Garbage Collected block: %d\n", selected_block_ids[i]);
        eraseBlock(selected_block_ids[i], 1);
    }
    
    vfree(blk_data_buf);
    return 0;

	// wear count : invalid block ratio
	// choose the block with the lowest ratio
	// invalid blocks can be zero so take care of that
	// move the data around
	// call the eraseblock function on that block 



// todo:
	// 		remove readonly flag after cleaning a block 
		// remove readonly flag after deleting a value
// 
		// when to write metadata
		// when to garbage collect

}

/**
 *
 */
int move_data(int block_id, int page_index, int num_pages_to_write, 
        char * buf) {

    int i;
    size_t retlen;
    uint64_t addr;
    char *key;
    char *val;

    if (buf == NULL) 
        return -1;

    key = (char *) vmalloc(config.page_size);
    val = (char *) vmalloc(config.page_size);

    if (key == NULL) 
        return -1;

    if (val == NULL) {
        vfree(key);
        return -1;
    }

    printk(GC_PREFIX "Writing %d pages in block %d\n", num_pages_to_write,
            block_id);

    for (i = 0; i < num_pages_to_write; i++) {
     	/* compute the flash target address in bytes */
	    addr = ((uint64_t) page_index + i) * ((uint64_t) config.page_size);
	    /* call the NAND driver MTD to perform the write operation */
	    if (config.mtd->
	        _write(config.mtd, addr, config.page_size, &retlen, 
                buf + (i * config.page_size)) != 0) {
	    	    vfree(key);
			    vfree(val);
            return -2;
	    }

        config.blocks[block_id].pages_states[(page_index + i) 
            % config.pages_per_block] = PG_VALID;

        /* update metadata in RAM here */
       // printk(GC_PREFIX "Updating metadata for key-val. Block: %d, Page: %d\n",
       //         block_id, page_index + i);
        
        read_key_val_from_page(key, val, buf + (i * config.page_size));
        update_key_val_metadata(key, block_id, i);

       //write_page(page_index + i, blk_data_buf + (i * config.page_size));
    }

    config.blocks[block_id].free_pages = config.pages_per_block - num_pages_to_write;
    config.current_block = block_id;
    config.current_page_offset = num_pages_to_write;
    /* Update free block's state to BLK_USED */
    config.blocks[block_id].state = BLK_USED;
    
    vfree(key);
    vfree(val);
    return 0;
}




/**
 * Returns the number of free blocks left in the flash
 */
int get_num_free_blocks() {
    int i;
    int num_free_blocks = 0;
    for (i = config.metadata_blocks; i < config.nb_blocks; i++) {
        if (config.blocks[i].state == BLK_FREE) 
            num_free_blocks++;   
    }
    return num_free_blocks;
}


/**
 * After an insertion, determine which is the flash page that will receive the 
 * next insertion. Return the correspondign flash page index, or -1 if the 
 * flash is full
 */
int get_next_page_index_to_write()
{
    int block;
	/* in general we want the next flash page in the block */
	config.current_page_offset++;

	/* but sometimes we need to jump to the next flash block */
	if (config.current_page_offset == config.block_size / config.page_size) {
        block = get_next_free_block();
	    
        /* GC has set the current block and page offset */
        if (block == -2)
            return -2;

        /* flash full */
        if (block == -1) {
            config.current_block = -1;
            return -1;	
        }
        
        config.current_block = block;
        config.blocks[config.current_block].state = BLK_USED;	
        if (config.current_page_offset == config.block_size / config.page_size)
            config.current_page_offset = 0;
	}
    return config.current_page_offset;
}

/**
 * When a flash block is full, we choose the next one through this function. 
 * Return the next block index, or -1 on error
 */
int get_next_free_block()
{
	int i;
	uint64_t min_wipeCount;
	uint64_t blockToChoose;
	printk(PRINT_PREF "*************In get_next_free_block \n");


	// todo check if < 20 percent blocks are free then call garbage collect
    if ((get_num_free_blocks() * 100) / config.nb_blocks <= 20) {
        garbage_collect();
        print_flash_state();
        return -2;
    }

    /* Here, we select a free block to write. We select a block that has
     * been erased the least number of times to manage wear levelling */
	//min_wipeCount = config.blocks[config.metadata_blocks].wipeCount;
    min_wipeCount = UINT_MAX;
	blockToChoose = config.metadata_blocks;

	for (i = config.metadata_blocks; i < config.nb_blocks; i++) {
		if (config.blocks[i].state == BLK_FREE) {
			if (config.blocks[i].wipeCount <= min_wipeCount) {
				blockToChoose = i;
			}
		}
	}
	if (config.blocks[blockToChoose].state == BLK_FREE) {
		printk(PRINT_PREF "*************Chosen block = %llu \n", blockToChoose);
		return blockToChoose;
	}
	printk(PRINT_PREF "*************Chosen block = -1\n");

	/* If we get there, no free block left... */

	// TODO: set config.current_page_offset if 
		// the the block is partially empty

	// todo point page offset to middle of the block if a block is partilly used
	// if blk.status = used but blk.freepages > 0

	// TODO: garbageCollect();

	min_wipeCount = config.blocks[config.metadata_blocks].wipeCount;
	blockToChoose = config.metadata_blocks;
	
	for (i = config.metadata_blocks; i < config.nb_blocks; i++) {
		if (config.blocks[i].state == BLK_FREE) {
			if (config.blocks[i].wipeCount < min_wipeCount) {
				blockToChoose = i;
			}
		}
	}
	if (config.blocks[blockToChoose].state == BLK_FREE)
		return blockToChoose;

	// choose middle of free block

	return -1;
}

/**
 * Callback for the erase operation done during the format process
 */
void format_callback(struct erase_info *e)
{
	if (e->state != MTD_ERASE_DONE) {
		printk(PRINT_PREF "Format error...");
		down(&config.format_lock);
		config.format_done = -1;
		up(&config.format_lock);
		return;
	}

	down(&config.format_lock);
	config.format_done = 1;
	up(&config.format_lock);
}

int eraseBlock(int blockNum, uint64_t number)
{
	//printk(PRINT_PREF "Erasing block %d\n", blockNum);

	struct erase_info ei;
	int i, j;

	if (number < 1) return 0;
	/* erasing one or several flash blocks is made through the use of an 
	 * erase_info structure passed to the MTD NAND driver */
	ei.mtd = config.mtd;
	ei.len = ((uint64_t) config.block_size) * ((uint64_t) number);
	ei.addr = ((uint64_t) config.block_size) * ((uint64_t) blockNum);
	/* the erase operation is made aysnchronously and a callback function will
	 * be executed when the operation is done */
	ei.callback = format_callback;

	config.format_done = 0;

	/* Call the MTD driver  */
	if (config.mtd->_erase(config.mtd, &ei) != 0)
		return -1;

	/* Wait for the operation completion with a spinlock, the callback function
	 * will set format_done to 1 
	 * here a better idea might be to use a condition variable
	 */
	while (1)
		if (!down_trylock(&config.format_lock)) {
			if (config.format_done) {
				up(&config.format_lock);
				break;
			}
			up(&config.format_lock);
		}

	/* was there a driver issue related to the erase oepration? */
	if (config.format_done == -1)
		return -1;
    
	/* update metadata: now all flash blocks and pages are free */
	for (i = blockNum; i < blockNum+number; i++) {
		config.blocks[i].state = BLK_FREE;
		config.blocks[i].wipeCount++;
		config.blocks[i].invalid_pages = 0;
		config.blocks[i].free_pages = config.pages_per_block;

		for (j = 0; j < config.pages_per_block; j++)
			config.blocks[i].pages_states[j] = PG_FREE;
	}

	//printk(PRINT_PREF "Block(s) erased\n");

	return 0;
}

/**
 * Format operation: we erase the entire flash partition
 */
int format()
{
	printk(PRINT_PREF "Formating.....\n");

	struct erase_info ei;
	int i, j;

	/* erasing one or several flash blocks is made through the use of an 
	 * erase_info structure passed to the MTD NAND driver */
	ei.mtd = config.mtd;
	ei.len = ((uint64_t) config.block_size) * ((uint64_t) config.nb_blocks - config.metadata_blocks);
	ei.addr = ((uint64_t) config.block_size) * ((uint64_t) config.metadata_blocks);
	/* the erase operation is made aysnchronously and a callback function will
	 * be executed when the operation is done */
	ei.callback = format_callback;

	config.format_done = 0;

	/* Call the MTD driver  */
	if (config.mtd->_erase(config.mtd, &ei) != 0)
		return -1;

	/* Wait for the operation completion with a spinlock, the callback function
	 * will set format_done to 1 
	 * here a better idea might be to use a condition variable
	 */
	while (1)
		if (!down_trylock(&config.format_lock)) {
			if (config.format_done) {
				up(&config.format_lock);
				break;
			}
			up(&config.format_lock);
		}

	/* was there a driver issue related to the erase oepration? */
	if (config.format_done == -1)
		return -1;
    
    /* update in-memory metadata */
    // config.dir.list = (directory_entry *) vmalloc((config.MAX_KEYS) * sizeof(directory_entry));

	for (i = 0; i < config.MAX_KEYS; ++i) {
		config.dir.list[i].keyHash = 0;
		config.dir.list[i].state = KEY_DELETED;
	}

	/* update metadata: now all flash blocks and pages are free */
	for (i = config.metadata_blocks; i < config.nb_blocks; i++) {
		config.blocks[i].state = BLK_FREE;
		config.blocks[i].wipeCount++;
		config.blocks[i].invalid_pages = 0;
		config.blocks[i].free_pages = config.pages_per_block;

		for (j = 0; j < config.pages_per_block; j++)
			config.blocks[i].pages_states[j] = PG_FREE;
	}

	// config.current_block = 0;
	config.current_page_offset = 0;
	// config.blocks[0].state = BLK_USED;
	config.read_only = 0;
	config.current_block = config.metadata_blocks;
	config.blocks[config.metadata_blocks].state = BLK_USED;

	printk(PRINT_PREF "Format done\n");
	writeFSMetadata();
	return 0;
}

/**
 * Write the flash page with index page_index, data to write is in buf. 
 * Returns:
 * 0 on success
 * -1 if we are in read-only mode
 * -2 when a write error occurs
 */
int write_page(int page_index, const char *buf)
{
	uint64_t addr;
	size_t retlen;
    int next_pg_index;

	/* if the flash partition is full, dont write */
	if (config.read_only)
		return -1;

	/* compute the flash target address in bytes */
	addr = ((uint64_t) page_index) * ((uint64_t) config.page_size);

	/* call the NAND driver MTD to perform the write operation */
	if (config.mtd->
	    _write(config.mtd, addr, config.page_size, &retlen, buf) != 0)
		return -2;

	/* update metadata to set the written page state, it is now valid */
	config.blocks[page_index /
		      config.pages_per_block].pages_states[page_index %
							   config.
							   pages_per_block] =
	    PG_VALID;

	/* set the flash page that will serve the next write oepration.
	 * if the flash partition is full, switch to read-only mode */
	next_pg_index = get_next_page_index_to_write();
    if (next_pg_index == -1) {
		printk(PRINT_PREF
		       "no free block left... switching to read-only mode\n");
		config.read_only = 1;
		return -1;
	} 
	return 0;
}

/**
 * Read the flash page with index page_index, data read are placed in buf
 * Retourne 0 when ok, something else on error
 */
int read_page(int page_index, char *buf)
{
	uint64_t addr;
	size_t retlen;
	// printk(PRINT_PREF "in read_page()\n");


	/* compute the flash target address in bytes */
	addr = ((uint64_t) page_index) * ((uint64_t) config.page_size);

	/* call the NAND driver MTD to perform the read operation */
	return config.mtd->_read(config.mtd, addr, config.page_size, &retlen,
				 buf);
}

int read_page_mtd(int page_index, char *buf, struct mtd_info *mtd)
{
	uint64_t addr;
	size_t retlen;
	// printk(PRINT_PREF "in read_page_mtd()\n");


	/* compute the flash target address in bytes */
	addr = ((uint64_t) page_index) * ((uint64_t) mtd->writesize);

	/* call the NAND driver MTD to perform the read operation */
	return mtd_read(mtd, addr, mtd->writesize, &retlen,
					 buf);
	// return mtd->_read(mtd, addr, mtd->writesize, &retlen,
	// 			 buf);
}


void writeFSMetadata(void) {
	uint64_t addr;
	size_t retlen;
	char *buffer;
	int numPages = 0;
	uint64_t size = 0;
	uint64_t j = 0;
	int i = 0;
	int k = 0;


	eraseBlock(0, config.metadata_blocks);


	size+= sizeof(lkp_kv_cfg);
	size+= sizeof(blk_info)* config.nb_blocks;
	size+= sizeof(page_state) * config.nb_blocks * 
		config.pages_per_block;

	for (j = 0; j < size;) {
		j += config.page_size;
		numPages += 1;
	}

	buffer = (char *)vmalloc(config.page_size * numPages);
	for (i = 0; i < config.page_size * numPages; i++)
		buffer[i] = 0x0;


	// Copy data to buffer

	memcpy(buffer, &config, sizeof(lkp_kv_cfg));

	for (i = 0; i < config.nb_blocks; ++i) {
		
		memcpy(buffer + sizeof(lkp_kv_cfg) 
			+ (i*sizeof(blk_info))
			+ (i*config.pages_per_block*sizeof(page_state))
			, &(config.blocks[i]), sizeof(blk_info));
		
		for (k = 0; k < config.pages_per_block; ++k) {
			
			memcpy(buffer + sizeof(lkp_kv_cfg) 
				+ ((i+1)*sizeof(blk_info))
				+ (i*config.pages_per_block*sizeof(page_state))
				+ (k*sizeof(page_state))
				, &(config.blocks[i].pages_states[k]),
			 sizeof(page_state));
		
		}
	}
	for (i = 0; i < numPages; ++i) {
		addr = ((uint64_t) i) * ((uint64_t) config.page_size);
		if (config.mtd->
		    _write(config.mtd, addr, config.page_size, &retlen, buffer + addr) != 0) {

				printk(PRINT_PREF "Error writing metadata\n");
				vfree(buffer);
				return;
		}
	}
	vfree(buffer);
	k = numPages;


	//  Write Directory
	numPages = 0;
	size = sizeof(directory_entry) * config.MAX_KEYS;
	for (j = 0; j < size;) {
		j += config.page_size;
		numPages += 1;
	}

	buffer = (char *)vmalloc(config.page_size * numPages);

	for (i = 0; i < config.page_size * numPages; i++)
		buffer[i] = 0x0;
	
	// copy dir to memory
	for (i = 0; i < config.MAX_KEYS; ++i) {

		memcpy(buffer + sizeof(directory_entry)*i
			, &(config.dir.list[i]), sizeof(directory_entry));
	}

	for (i = 0; i < numPages; ++i) {
		addr = ((uint64_t) k + i) * ((uint64_t) config.page_size);
		if (config.mtd->
		    _write(config.mtd, addr, config.page_size, &retlen, buffer + (i*config.page_size)) != 0) {

				printk(PRINT_PREF "Error writing metadata\n");
				vfree(buffer);
				return;
		}
	}

	vfree(buffer);
}


/**
 * Print some statistics on the kernel log
 */
void print_config()
{
	printk(PRINT_PREF "Config : \n");
	printk(PRINT_PREF "=========\n");

	printk(PRINT_PREF "mtd_index: %d\n", config.mtd_index);
	printk(PRINT_PREF "nb_blocks: %d\n", config.nb_blocks);
	printk(PRINT_PREF "block_size: %d\n", config.block_size);
	printk(PRINT_PREF "page_size: %d\n", config.page_size);
	printk(PRINT_PREF "pages_per_block: %d\n", config.pages_per_block);
	printk(PRINT_PREF "metadata_blocks: %d\n", config.metadata_blocks);
	printk(PRINT_PREF "read_only: %d\n", config.read_only);
}

/**
 * Print the state of the flash
 */
void print_flash_state() {
    int i;
    printk(PRINT_PREF "Flash State : \n");
    printk(PRINT_PREF "==============\n");
    for (i = 0; i < config.nb_blocks; i++) {
        printk(PRINT_PREF "Block: ID: %d - ", i);
        printk(PRINT_PREF "STATE: %d (0 = free, 1 = used) - ", config.blocks[i].state);
        printk(PRINT_PREF "Invalid Pages: %d - ", config.blocks[i].invalid_pages);
        printk(PRINT_PREF "Free Pages: %llu - ", config.blocks[i].free_pages);
        printk(PRINT_PREF "Erase Count: %llu\n", config.blocks[i].wipeCount);
    }
}

/* Setup init and exit functions */
module_init(lkp_kv_init);
module_exit(lkp_kv_exit);

/**
 * Infos generale sur le module
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pierre Olivier <polivier@vt.edu>");
MODULE_DESCRIPTION("LKP key-value store prototype");
