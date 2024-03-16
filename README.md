# A concurrent counting task

In this assignment, one or more threads are collaborating on a task related to shared buffers. Start off by cloning this repo to *nodes*, and running ``make``. Run the program with ``./bufcount``. You can also run a few example configurations with ``make eval``.

The program performs two main tasks. Updating an item count, for each of the items, and updating the buffer, for each of the items. Your task is to make this program run fast, per iteration, for both of these tasks. 

The way you get the program, it runs with a single thread, and takes about 7 ns per count update, and 12 per buffer update. The client understandably finds this unacceptable. They demand much faster single-threaded item counts. They make no demands about the buffer updates.

To make matters worse, when you increase the thread count from 1 to 16, things get really impractically slow. Item counts go up to 13 ns, and buffer counts hit 22 ns. The client understandably expects the program to run faster with more cores, but the way things are going, there are some strongly diminishing returns with more threads. It's *even* worse when you shrink ARRAY_SIZE from the default of 128 to a more modest 16. Not to mention 1.

The client expects the program to perform really well for THREAD_MAX betwen 1 and 16, and for ARRAY_SIZE between 1 and 128, and they don't want to see you wasting memory.

## Some context

We do want to actually perform the work - in this assignment, we're not looking for cleverly computing the final value and assigning this, or having the compiler optimize the work out. There are a couple of files marked "donottouch". Feel free to read these, but the intent is for these files to provide a constraint on what you can and cannot do. 

There are no hidden gotchas in the program, no weird macros or constants you need to discover and change. Instead, your job is to preserve the "spirit" of the program (in addition to the output), but make it run faster by thinking about and experimenting with how memory accesses affect performance. 

## Single-threaded

Start exploring this problem by getting single-threaded execution to run better. Keep in mind that the client was mostly concerned about the count update measurement here. You should be able to get that down to 2-3 ns. 

### malloc the bufs
If you look at the item struct, you'll find that there's a buffer allocated inside the struct. Try allocating this separately with ``malloc`` instead. You'll need to change the item struct, that's fine. You'll find that although it compiles and runs, this approach fails a runtime test. Try to figure out why malloc does the wrong thing.

### memalign the bufs
Instead of malloc, try using memalign. Pay attention to the heap size output from the malloc approach, vs. the output from the memalign approach. Try to figure out why memalign uses so much more memory than malloc. 

Why does the item count run so much faster now?

### statically allocate the bufs separately
Instead of memalign, try sticking the bufs in a statically allocated, separate array, and having the buf pointer in the item point to the appropriate item in this array. 

How does the performance of this compare to the performance of the memalign version? Is there a statistically significant difference?
What if you update ARRAY_SIZE to 128 instead of 32? Now you should see a substantial difference. Why does the memalign vs array allocation only make a big impact for ARRAY_SIZE 128? Once you form a hypothesis, try `perf stat -M Cache_Misses`. It measures a handy group of counters.
Also, see if your hypothesis can predict something about ARRAY_SIZE 64.

## Multi-threaded

Change THREAD_MAX from 1 to 4, or 16. When running with more than 1 thread, a spinlock is used to protect shared data. Unfortunately, with 16 threads, operations take almost 10x as long.
The client wants to see much better scalability, in particular for the buffer update.

Here are some things to try. For the best learning experience, try them in order.

### switch to a mutex lock

With a spinlock, threads continuously try to grab the lock until they get it. This causes congestion. With mutex, the thread goes to sleep and is woken up when the lock is available. This, instead, causes system calls. Which one is the better choice? Try varying ARRAY_SIZE (number of locks, in this case) and MAX_THREADS to see if there are cases where one is better, and other cases when the other is better.

Try running both cases with `strace -cfw`, to see how many system calls mutex creates. The `-f` is key here, since you want to trace the new threads we're creating, not just the main thread.  

### observe some NUMA effects

Now is a good time to have a look at the effect of non-uniform memory access times. When you simply run ./bufcounts, the OS scheduler will schedule threads on a random-looking mix of cores. Try running it with `htop` running in a separate window, to see what cores are working. 

Try restricting which cores the program can use with taskset: `taskset -c 0-15 ./bufcounts` vs. `taskset -c 0-7,16-23 ./bufcounts`. The first choice selects cores all in the same socket. The second one selects 8 on one socket, and 8 on the next one over. Note that we aren't actually forcing the threads onto all these cores, but merely restricting the scheduler to using these. In principle, it could schedule all of them on a single core. 

How does performance vary with core assignment? What if you pick cores that are two interconnect hops apart? (say 0-7 and 32-39)?

### think about alignment and false sharing

False sharing (two separate data items sitting in the same cache line) can be a very big performance factor. 
If you force the items to be on separate cache lines (try adding a char array of the right size to the item), does that help? 
What about if you keep them 128 bytes apart?

### switch to an atomic count

For the counting part, you don't actually need a lock, you can use atomic operations on the integer instead. (such as 
__sync_fetch_and_add(), https://gcc.gnu.org/onlinedocs/gcc-4.1.2/gcc/Atomic-Builtins.html). How much does this help?

### drop the lock for buffer updates

Try simply dropping the lock for the buffer updates. Surely this will create race conditions and invalid results. How does it impact performance?

### change the buffer update logic a bit

Right now, every thread updates the first number in a buffer's count array. Since they share the count, they need a lock. But the validation code near the end of main only checks that the *sum* of all the counts is correct. If you pass the thread ID to update_buffer, it'll increment a different number in counter array. With different counters, you don't need locking.

### a buf has 128 counters, can you use that?

If you space out the thread IDs so that they increment counts that are on different cache lines, does that help performance?
Each counter is 8 bytes, so if you space the ID's out by 8, they'll end up on a 64-byte cache line each, eliminating cache contention. 

### what's happening between 32 and 64 items for the buffer update?

Once your solution runs in the single ns per update with 16 threads, you may find that it goes a great deal slower with 64 items than with 32 items. Can you work out why?

## Turn-in

Please post and discuss your results on Piazza. 
