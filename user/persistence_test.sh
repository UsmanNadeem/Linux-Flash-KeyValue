#!/bin/bash

PREF="[ TEST_SCRIPT ]: "

echo -e "$PREF Launching Tests for Flash:"

echo -e "$PREF Inserting the module"
./insert_mod.sh

echo -e "$PREF Launching Testbench"
./testbench_modif

echo -e "$PREF Removing the module"
rmmod prototype.ko

echo -e "$PREF Executing persistence tests"

echo -e "$PREF Inserting the module"
./insert_mod.sh

echo -e "$PREF Writing to flash"
./test_write_persistence 0

echo -e "$PREF Removing the module"
rmmod prototype.ko

echo -e "$PREF Re-inserting the moduel"
./insert_mod.sh

echo -e "$PREF Reading from flash"
./test_write_persistence 1

