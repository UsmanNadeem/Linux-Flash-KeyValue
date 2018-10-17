/**
 * Header for the protoype main file
 */

#ifndef LKP_KV_H
#define LKP_KV_H

#include <linux/mtd/mtd.h>
#include <linux/semaphore.h>

/* state for a flash block: used or free */
typedef enum {
	BLK_FREE,
	BLK_USED
} blk_state;

/* state for a flash page: used or free */
typedef enum {
	PG_FREE,
	PG_VALID,
	PG_DELETED
} page_state;

/* state for a key: valid or deleted */
typedef enum {
	KEY_DELETED,
	KEY_VALID
} key_state;

/* data structure containing the state of a flash block and the state of the 
 * flash pages it contains */
typedef struct {
	blk_state state;
	uint64_t wipeCount;
	uint64_t free_pages;  // free == available to write
	unsigned int invalid_pages;	/* number of invalid pages in block. Needed for GC victim block selection */
	page_state *pages_states;
} blk_info;

// mapping and state for a single key 
typedef struct {
	__u32 keyHash; 
	int block;
	key_state state;	// if deleted we can overwrite the directory_entry list index with another key.
						// we can only have a limited number of keys.
	int page_offset;
} directory_entry;

// list of keys
typedef struct {
	directory_entry* list;	// todo size == MAX_numkeys == Total numPages
							// can be converted to a list
} directory;

/* global attributes for our system */
typedef struct {
	struct mtd_info *mtd;	/* pointer to the used flash partition mtd_info object */
	int mtd_index;		/* the partition index */
	int nb_blocks;		/* amount of managed flash blocks */
	int block_size;		/* flash bock size in bytes */
	int page_size;		/* flash page size in bytes */
	int current_block;	/* current flash block we write to */
	int current_page_offset;	/* index of the next flash page to write in the current block */
	int pages_per_block;	/* number of flash pages per block */
	blk_info *blocks;	/* metadata : flash blocks/pages state */
	int format_done;	/* used during format operation */
	int read_only;		/* are we in read-only mode? */
	struct semaphore format_lock;	/* used during the format operation */

	directory dir;	/* mapping from key to block */
	uint64_t MAX_KEYS;
	unsigned int metadata_blocks;
    int invoke_gc;
} lkp_kv_cfg;

/* export some prototypes for function used in the virtual device file */
int set_keyval(const char *key, const char *val);
int get_keyval(const char *key, char *val);
int del_keyval(const char *key);
int update_keyval(const char *key, const char *val);
int format(void);

extern lkp_kv_cfg config;

#endif /* LKP_KV_H */
