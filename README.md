In-kernel key-value storage system prototype
LKP Project 06
---------------

The "kernel" folder contains all kernel code, in the form of a module. That 
code can be divided into two parts: the module core functions (in "core.h"
and "core.h") containing the storage system initialization and exit functions,
as well as the implementation of the storage system algorithms. In "device.c"
and "device.h", the code related to the virtual device management is located.

The "user" folder contains all userspace related code. "kvlib.c" and "kvlib.h"
are the sources of the library with wich you must compile the program that 
needs to access the storage system. "testbench.c" is an example of such a 
program containing a set of test cases.


The user folder also contains all the test files for the project.
Here is the list of test files and their functionalities.

1. testbench_kvfs.c             ==>     Validation tests for our KV flash storage
   (Execute using: ./testbench_kvfs)
2. test_write_persistence.c     ==>     Tests for persistence (single int argument)
                                        if arg == 1, formats and writes
                                        if arg == 2, reads the written flash
  Above files (1 and 2) should be launched with the bash script persistence_test.sh

3. wearlevel.c                  ==>     Tests and validates wear leveling
   (Execute using: ./wearlevel)

Performance Testing:
====================
All parameters (incl block size) are hardcoded in the files.

1. performance.c                ==>     Measures time taken for write (25%, 50% and
                                        100% of the disk), sequential_read and 
                                        random_read to the flash
  (Launch using: ./performance)
2. performance2.c               ==>     Measures the performance of the KV store
                                        for write, read, update and delete at
                                        25%, 50%, 75% and 100% capacity of the 
                                        flash storage.
  (Launch using: ./performance2)


====================

The script copy_to_vm.sh can be used to transfer all the required files to the VM
for testing.
