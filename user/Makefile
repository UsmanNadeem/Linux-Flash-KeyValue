CC=gcc
CFLAGS=-Wall -Werror
LDFLAGS=
TARGET=root@192.168.53.89:~

all: kvlib.o testbench testbench_modif

kvlib.o: kvlib.c
	$(CC) $(CFLAGS) -c $^ -Wall -o $@ $(LDFLAGS)
	
testbench: testbench.c kvlib.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

testbench_modif: testbench_modif.c kvlib.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)


install: all
	scp -r launch_flash_simulator.sh testbench $(TARGET)

clean:
	rm -rf *.o testbench
