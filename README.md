# memoryPool
A memory pool written in c++, to reduce the possiblity of memory fragments and increase memory alloc performance

## Compile and Run
Visual studio 2013, and it should work in linux.
std::mutex is required if enabling MEMORY_ENABLE_MULTI_THREAD_ALLOC macro.

## Where did the code comes from
Sorry I forgot it. It have been a long story when I am interested in memory pool. I grasp some codes and they now live in my HDD. These days, when I cleaned up my codes, I found it and made some modification for more common use.

## What is the basic idea
First we have many memory slots, where each slot is a link list of memory blocks with same size corresponding to the slot index.

The slot number: MEMORT_LENGTH_MAX/MEMORY_LENGTH_STEP
block size in slot i: (i+1)*MEMORY_LENGTH_STEP
max_meory_block_size:  MEMORT_LENGTH_MAX

When you request a memory with size n, it will first find the slot index ( (n-1)/MEMORY_LENGTH_STEP), and return one block in this slot. If there are no blocks any more in this slot, it will new one block with the size of n (not accurately, maybe larger) and return back.

When you return back a memory block with point ptr, it will first calculate the block size by moving backward 8 bytes, and get the slot index by block size. Then a new block is inserted into this slot, which is available for futher use.

## Functions
In initialization phase, it automatically alloc NUM_POPULAR blocks with the size of MEMORY_LENGTH_POPULAR (in fact, usually more than MEMORY_LENGTH_STEP, exactly divisible by MEMORY_LENGTH_STEP)

You also add your custom popular size by function `AddPopularMemory`


