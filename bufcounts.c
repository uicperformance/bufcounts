#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <malloc.h>
#include "donottouch.h"
#define ARRAY_SIZE 128
#define THREAD_MAX 1
#define ITERATIONS 100000

typedef struct {
    pthread_spinlock_t lock;
    buf buffer;
} item;
item items[ARRAY_SIZE]; // Array of structs


// Function to increment the first element of each buffer
void update_buffer_elements(unsigned thread_id) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
//        pthread_spin_lock(&items[i].lock);
        update_buffer(&items[i].buffer,thread_id);
//        pthread_spin_unlock(&items[i].lock);
    }
}

void* thread_function(void* arg) {
    int identifier = (int)(long)arg;  // Cast back to int

    for (int i = 0; i < ITERATIONS; i++) {
        update_buffer_elements(identifier);
    }
    return NULL;
}

int main() {
    struct timespec start, end;
    long long int elapsed_ns;
    int correct = 1;
    unsigned long expected_value = ITERATIONS;

    // Initialize pthread_spinlock_t for each item
    for (int i = 0; i < ARRAY_SIZE; i++) {
        memset(items[i].buffer.array,0,BUF_SIZE*8);
        pthread_spin_init(&items[i].lock, 0);
    }

    malloc_trim(0);
    struct mallinfo2 info = mallinfo2();
    printf("Heap size is %lu after allocating buffers. Buffers are %u total.\n",info.arena,ARRAY_SIZE*BUF_SIZE*8);


    pthread_t threads[THREAD_MAX];

    clock_gettime(CLOCK_MONOTONIC, &start);
        for (int i = 0; i < THREAD_MAX; i++) {
        if (pthread_create(&threads[i], NULL, thread_function, (void*)(long)i) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    for (int i = 0; i < THREAD_MAX; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Failed to join thread");
            return 1;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    // Check the first elements in all buf arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < THREAD_MAX; j++) {
            if (items[i].buffer.array[j] != expected_value) {
                printf("Error: Item %d, element %d, has value %ld != %ld\n",
                    i,j,items[i].buffer.array[j],expected_value);
                correct = 0;
                break;
            }
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
        printf("Each increment took %lld nanoseconds.\n", elapsed_ns / (ARRAY_SIZE * ITERATIONS));
    } else {
        printf("Error: Buffer values are incorrect.\n");
    }

    // Clean up the spin locks
    for (int i = 0; i < ARRAY_SIZE; i++) {
        pthread_spin_destroy(&items[i].lock);
    }

    return 0;
}
