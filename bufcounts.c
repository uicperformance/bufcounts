#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <malloc.h>
#include "donottouch.h"
#ifndef ARRAY_SIZE
#define ARRAY_SIZE 32
#endif

#ifndef THREAD_MAX
#define THREAD_MAX 1
#endif

#ifndef ITERATIONS
#define ITERATIONS 100000
#endif

typedef struct {
    pthread_spinlock_t lock;
    long total_count;
    buf buffer;
} item;
item items[ARRAY_SIZE]; 

void update_buffer_elements(unsigned thread_id) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
    #if THREAD_MAX > 1
        pthread_spin_lock(&items[i].lock);
    #endif
        update_buffer(&items[i].buffer,0);
    #if THREAD_MAX > 1
        pthread_spin_unlock(&items[i].lock);
    #endif
    }
}

void update_item_counts(unsigned thread_id) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
    #if THREAD_MAX > 1
        pthread_spin_lock(&items[i].lock);
    #endif
        items[i].total_count++;
    #if THREAD_MAX > 1
        pthread_spin_unlock(&items[i].lock);
    #endif
    }
}

void* thread_function(void* arg) {
    int identifier = (int)(long)arg;  // Cast back to int

    for (int i = 0; i < ITERATIONS; i++) {
        update_buffer_elements(identifier);
    }
    return NULL;
}

void* thread_function2(void* arg) {
    int identifier = (int)(long)arg;  // Cast back to int

    for (int i = 0; i < ITERATIONS; i++) {
        update_item_counts(identifier);
    }
    return NULL;
}


int main() {
    struct timespec start_buffer, end_buffer, start_item, end_item;
    int correct = 1;

    // Initialize pthread_spinlock_t for each item
    for (int i = 0; i < ARRAY_SIZE; i++) {
        memset(items[i].buffer.counter,0,BUF_SIZE*8);
        pthread_spin_init(&items[i].lock, 0);
    }

    malloc_trim(0);
    struct mallinfo2 info = mallinfo2();
    printf("Heap size is %lu after allocating buffers. Buffers are %u total.\n",info.arena,ARRAY_SIZE*BUF_SIZE*8);
    pthread_t threads[THREAD_MAX];
    clock_gettime(CLOCK_MONOTONIC, &start_buffer);
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
    clock_gettime(CLOCK_MONOTONIC, &end_buffer);

    clock_gettime(CLOCK_MONOTONIC, &start_item);
        for (int i = 0; i < THREAD_MAX; i++) {
        if (pthread_create(&threads[i], NULL, thread_function2, (void*)(long)i) != 0) {
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
    clock_gettime(CLOCK_MONOTONIC, &end_item);

    // Check that things match up
    for (int i = 0; i < ARRAY_SIZE; i++) {
        long sum = 0;
        for (int j = 0; j < BUF_SIZE; j++) {
            sum+=items[i].buffer.counter[j];
        }
        if (sum != items[i].total_count) {
            printf("Error: Item %d has sum %ld != %ld. Expected %d.\n",
                i,sum,items[i].total_count,THREAD_MAX*ITERATIONS);
            correct = 0;
            break;
        }

        if(check_buffer_alignment(&items[i].buffer)!=0) {
            printf("Error: Buffer is incorrectly aligned %p, should be aligned %lu.\n",
            &items[i].buffer, alignof(buf));
        }
    }

    // Calculate elapsed time in nanoseconds
    long elapsed_ns_buf = (end_buffer.tv_sec - start_buffer.tv_sec) * 1000000000LL + (end_buffer.tv_nsec - start_buffer.tv_nsec);
    long elapsed_ns_item = (end_item.tv_sec - start_item.tv_sec) * 1000000000LL + (end_item.tv_nsec - start_item.tv_nsec);

    // Output the result
    if (correct) {
        printf("Running with %d threads, %d items, %d iterations.\n",THREAD_MAX, ARRAY_SIZE, ITERATIONS);
        printf("Each item count increment took %.2lf nanoseconds.\n", elapsed_ns_item / (double)(ARRAY_SIZE * ITERATIONS * THREAD_MAX));
        printf("Each buffer update took %.2lf nanoseconds.\n", elapsed_ns_buf / (double)(ARRAY_SIZE * ITERATIONS * THREAD_MAX));
    } else {
        printf("Error: Buffer values are incorrect.\n");
    }

    // Clean up the spin locks
    for (int i = 0; i < ARRAY_SIZE; i++) {
        pthread_spin_destroy(&items[i].lock);
    }

    return 0;
}
