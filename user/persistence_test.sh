#!/bin/bash
echo -e "Inserting the module"
./insert_mod.sh

echo -e "Writing to flash"
./test_write_persistence 0

echo -e "Removing the module"
rmmod prototype.ko

echo -e "Re-inserting the moduel"
./insert_mod.sh

echo -e "Reading from flash"
./test_write_persistence 1

