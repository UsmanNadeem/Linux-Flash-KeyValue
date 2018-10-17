#!/bin/bash

# Script launching the nandsim flash simulator

set -euo pipefail

# id_bytes parameter description:
# -------------------------------
# The combination of the id bytes represents a given flash chip model in terms
# of flash page size, number of flash pages per block, and total flash chip 
# capacity. The parameters currently set describe a 256MB flash chip with 2KB
# pages and 64 pages per block
# More info on id_bytes: 
# http://www.linux-mtd.infradead.org/faq/nand.html#L_nand_nandsim

# parts parameter description:
# ----------------------------
# This parameter is an array of integers separated with comas. It allows 
# partitionning the flash chip, each integer giving the size, _in eraseblocks_,
# of a partition (first integer -> partition index 0, second integer -> 
# partition index 1, etc.)
# If the total size of described partition is inferior to the size of the flash
# chip, a last partition is automatically created to pad until the end of the 
# chip.
# Note that when a partition is created, it is presented by the driver as a 
# standalone device (there is an entry /dev/mtdX, X being the partition index),
# and there is a unique object mtd_info in the kernel related to this 
# partition)

sudo modprobe nandsim first_id_byte=0x20 second_id_byte=0xaa third_id_byte=0x00 fourth_id_byte=0x15 parts=10,800 access_delay=25 programm_delay=200 erase_delay=1
