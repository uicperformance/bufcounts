#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <malloc.h>
#include "donottouch.h"
#define ARRAY_SIZE 128

typedef struct {
    pthread_spinlock_t lock;
    buf buffer;
} item;


// Function to increment the first element of each buffer
void update_buffer_elements(item items[],unsigned thread_id) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        update_buffer(&items[i].buffer,thread_id);
    }
}

int main() {
    item items[ARRAY_SIZE]; // Array of structs
    struct timespec start, end;
    long long int elapsed_ns;
    int correct = 1;
    unsigned long expected_value = 0;

    // Initialize pthread_spinlock_t for each item
    for (int i = 0; i < ARRAY_SIZE; i++) {
        memset(items[i].buffer.array,0,BUF_SIZE*8);
        pthread_spin_init(&items[i].lock, 0);
    }

    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Repeat the increment operation to get a measurable time
    for (int i = 0; i < 1000000; i++) {
        update_buffer_elements(items,0);
        expected_value++;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    struct mallinfo2 info = mallinfo2();
    printf("Malloc is using %lu bytes. Buffers are %u total.\n",info.arena,ARRAY_SIZE*BUF_SIZE);

    // Check the first elements in all buf arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (items[i].buffer.array[0] != expected_value) {
            printf("Error: Item %d has value %ld != %ld\n",
                i,items[i].buffer.array[0],expected_value);
            correct = 0;
            break;
        }
        if(check_buffer_alignment(&items[i].buffer)!=0) {
            printf("Error: Buffer is incorrectly aligned %p, should be aligned %lu.\n",
            &items[i].buffer, alignof(buf));
        }
    }

    // Calculate elapsed time in nanoseconds
    elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);

    // Output the result
    if (correct) {
        printf("Each increment took %lld nanoseconds.\n", elapsed_ns / (ARRAY_SIZE * 1000000LL));
    } else {
        printf("Error: Buffer values are incorrect.\n");
    }

    // Clean up the spin locks
    for (int i = 0; i < ARRAY_SIZE; i++) {
        pthread_spin_destroy(&items[i].lock);
    }

    return 0;
}
