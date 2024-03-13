CC=gcc
CFLAGS=-Wall -pthread
TARGET=bufcounts

bufcounts: bufcounts.c donottouch.c 
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
