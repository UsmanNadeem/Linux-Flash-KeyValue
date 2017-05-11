CC=gcc
CFLAGS=-Wall -Werror
LDFLAGS=
TARGET=root@192.168.53.89:~

all: kvlib.o testbench testbench_kvfs test_write_persistence performance performance2 writeFull wearlevel

kvlib.o: kvlib.c
	$(CC) $(CFLAGS) -c $^ -Wall -o $@ $(LDFLAGS)
	
testbench: testbench.c kvlib.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

testbench_kvfs: testbench_kvfs.c kvlib.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_write_persistence: test_write_persistence.c kvlib.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

performance: performance.c kvlib.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

performance2: performance2.c kvlib.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

writeFull: writeFull.c kvlib.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

wearlevel: wearlevel.c kvlib.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

install: all
	scp -r launch_flash_simulator.sh testbench $(TARGET)

clean:
	rm -rf *.o testbench
