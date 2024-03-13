# A concurrent counting task

In this assignment, one or more threads are collaborating on a task related to shared buffers. Start off by cloning this repo to *nodes*, and running ``make``. Run the program with ``./bufcount``. 

The program performs two main tasks. Updating an item count, for each of the items, and updating the buffer, for each of the items. Your task is to make this program run fast, per iteration, for both of these tasks. 

The way you get the program, it runs with a single thread, and takes about 7 ns per count update, and 7 per buffer. The client understandably finds this unacceptable. They demand much faster single-threaded item counts. They make no demands about the buffer updates.

To make matters worse, when you increase the thread count from 1 to 16, things get really impractically slow. Item counts go up to 13 ns, and buffer counts hit 22 ns. The client understandably expects the program to run faster with more cores, but the way things are going, there are some strongly diminishing returns with more threads. It's *even* worse when you shrink ARRAY_SIZE from the default of 128 to a more modest 16. Not to mention 1.

The client expects the program to perform really well for THREAD_MAX betwen 1 and 16, and for ARRAY_SIZE between 1 and 128.  
