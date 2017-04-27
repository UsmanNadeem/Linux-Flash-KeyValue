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


	if (readFSMetadata(mtd_index) == -1) {
		printk(PRINT_PREF "Init metadata read error creating new metadata. Assuming everything is formated\n");
		return initialize_firsttime(mtd_index);
	}




	return 0;
}


int readFSMetadata(int mtd_index) {

	/* The flash partition is manipulated by caling the driver, through the
	 * mtd_info object. There is one of these object per flash partition */
	struct mtd_info *mtd = get_mtd_device(NULL, mtd_index);

	if (mtd == NULL)
		return -1;
	int block_size = mtd->erasesize;
	int page_size = mtd->writesize;
	int pages_per_block = block_size / page_size;
	uint64_t tmp_blk_num = mtd->size;
	do_div(tmp_blk_num, (uint64_t) config.mtd->erasesize);
	int nb_blocks = (int)tmp_blk_num;




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


	buffer = (char *)vmalloc(config.page_size * numPages);
	for (i = 0; i < config.page_size * numPages; i++)
		buffer[i] = 0x0;


    /* read page */
	for (i = 0; i < numPages; ++i) {
		uint64_t address = ((uint64_t) i) * ((uint64_t) config.page_size);
	    if (read_page(address, buffer + address) != 0) {
			printk(PRINT_PREF "Error in readFSMetadata\n");
			vfree(buffer);
			return -1;
		}
	}


	memcpy(&config, buffer, sizeof(lkp_kv_cfg));


	if (config.mtd_index != mtd_index)
		return -1;

	config.mtd = get_mtd_device(NULL, mtd_index);

	if (config.mtd == NULL || config.nb_blocks != nb_blocks || config.block_size != block_size)
		return -1;

	config.blocks = (blk_info *) vmalloc((config.nb_blocks) * sizeof(blk_info));



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


    /* read page */
	for (i = 0; i < numPages; ++i) {
		uint64_t address = ((uint64_t) tmp_blk_num+i) * ((uint64_t) config.page_size);
		uint64_t baseOffset = ((uint64_t) i) * ((uint64_t) config.page_size);
	    if (read_page(address, buffer + baseOffset) != 0) {
			printk(PRINT_PREF "Error in readFSMetadata\n");
			vfree(buffer);
			return -1;
		}
	}

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
		config.blocks[i].pages_states =
		    (page_state *) vmalloc(config.pages_per_block *
					   sizeof(page_state));
		for (j = 0; j < config.pages_per_block; j++)
			config.blocks[i].pages_states[j] = PG_FREE;
	}


	int numPages = 0;
	uint64_t size = 0;

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
	config.current_block = i;
	config.current_page_offset = 0;

	config.dir.list = (directory_entry *) vmalloc((config.MAX_KEYS) * sizeof(directory_entry));

	for (i = 0; i < config.MAX_KEYS; ++i) {
		config.dir.list[i].keyHash = 0;
		config.dir.list[i].state = KEY_DELETED;
	}

	return 0;
}

/**
 * Freeing stuff on exit
 */
void destroy_config(void)  //todo fix
{
	int i;
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
	int key_len, val_len, i, ret;
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
				printk(PRINT_PREF "Key \"%s\" already exists Updating it\n", key);
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
	dirToAdd->block = config.current_block;
	dirToAdd->page_offset = config.current_page_offset;

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
	uint64_t address;
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
                printk(PRINT_PREF "Key \"%s\" found\n", key);
                entry = &config.dir.list[i];
                break;
		    }
		}
    }

	if (entry) {
        page_index = (entry->block * config.pages_per_block) + entry->page_offset;
        address = ((uint64_t) page_index) * ((uint64_t) config.page_size);

        /* read page */
        if (read_page(address, buffer) != 0) {
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
        WARN_ON(ret != 2);
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
 * After an insertion, determine which is the flash page that will receive the 
 * next insertion. Return the correspondign flash page index, or -1 if the 
 * flash is full
 */
int get_next_page_index_to_write()
{
	/* in general we want the next flash page in the block */
	config.current_page_offset++;

	/* but sometimes we need to jump to the next flash block */
	if (config.current_page_offset == config.block_size / config.page_size) {
		config.current_block = get_next_free_block();

		/* flash full */
		if (config.current_block == -1)
			return -1;

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

	for (i = config.metadata_blocks; i < config.nb_blocks; i++)
		if (config.blocks[i].state == BLK_FREE)
			return i;

	/* If we get there, no free block left... */

	// todo set config.current_page_offset if 
		// the the block is partially empty
	// todo garbageColect();
	for (i = config.metadata_blocks; i < config.nb_blocks; i++)
		if (config.blocks[i].state == BLK_FREE)
			return i;

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

/**
 * Format operation: we erase the entire flash partition
 */
int format()
{
	struct erase_info ei;
	int i, j;

	/* erasing one or several flash blocks is made through the use of an 
	 * erase_info structure passed to the MTD NAND driver */
	ei.mtd = config.mtd;
	ei.len = ((uint64_t) config.block_size) * ((uint64_t) config.nb_blocks);
	ei.addr = 0x0;
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
	for (i = 0; i < config.nb_blocks; i++) {
		config.blocks[i].state = BLK_FREE;
		for (j = 0; j < config.pages_per_block; j++)
			config.blocks[i].pages_states[j] = PG_FREE;
	}

	config.current_block = 0;
	config.current_page_offset = 0;
	config.blocks[0].state = BLK_USED;
	config.read_only = 0;

	printk(PRINT_PREF "Format done\n");

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
	if (get_next_page_index_to_write() == -1) {
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

	/* compute the flash target address in bytes */
	addr = ((uint64_t) page_index) * ((uint64_t) config.page_size);

	/* call the NAND driver MTD to perform the read operation */
	return config.mtd->_read(config.mtd, addr, config.page_size, &retlen,
				 buf);
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
	printk(PRINT_PREF "read_only: %d\n", config.read_only);
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
