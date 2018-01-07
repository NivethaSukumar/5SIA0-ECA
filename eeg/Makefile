NUM_THREADS?=1

all:
	arm-linux-gnueabi-gcc -Wall -DNUM_THREADS=$(NUM_THREADS) -fopenmp C/*.c -static -lc -lm -lgomp pthread.o -o eeg.arm -lgomp

native:
	gcc -Wall -DNUM_THREADS=$(NUM_THREADS) -fopenmp C/*.c -lc -lm -o eeg

clean:
	rm eeg.arm eeg
