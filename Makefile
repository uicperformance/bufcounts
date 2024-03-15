CC=gcc
CFLAGS=-O3 -Wall -pthread
TARGET=bufcounts

bufcounts: bufcounts.c donottouch.c 
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
