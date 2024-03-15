CC=gcc
CFLAGS=-g -O3 -Wall -pthread
TARGET=bufcounts
.SILENT: 

bufcounts: bufcounts.c donottouch.c 
	$(CC) $(CFLAGS) -o $@ $^

eval: 
	$(CC) -DTHREAD_MAX=1 -DARRAY_SIZE=32 $(CFLAGS) -o bufcounts bufcounts.c donottouch.c 
	./bufcounts
	$(CC) -DTHREAD_MAX=16 -DARRAY_SIZE=1  $(CFLAGS) -o bufcounts bufcounts.c donottouch.c 
	./bufcounts
	$(CC) -DTHREAD_MAX=16 -DARRAY_SIZE=2  $(CFLAGS) -o bufcounts bufcounts.c donottouch.c 
	./bufcounts
	$(CC) -DTHREAD_MAX=16 -DARRAY_SIZE=4  $(CFLAGS) -o bufcounts bufcounts.c donottouch.c 
	./bufcounts
	$(CC) -DTHREAD_MAX=16 -DARRAY_SIZE=32  $(CFLAGS) -o bufcounts bufcounts.c donottouch.c 
	./bufcounts
	$(CC) -DTHREAD_MAX=16 -DARRAY_SIZE=64  $(CFLAGS) -o bufcounts bufcounts.c donottouch.c 
	./bufcounts
	$(CC) -DTHREAD_MAX=16 -DARRAY_SIZE=128  $(CFLAGS) -o bufcounts bufcounts.c donottouch.c 
	./bufcounts

clean:
	rm -f $(TARGET)
