#ifndef MEMORYALLOCATOR_MEMORYALLOCATOR_H
#define MEMORYALLOCATOR_MEMORYALLOCATOR_H

#include <stdlib.h> /*for malloc and size_t*/

typedef struct MemoryAllocator MemoryAllocator;

/* memoryPool is a ptr to an already-existing large memory block */
MemoryAllocator* memoryAllocatorInit(void* memoryPool, size_t size);

/* Returns a ptr to the memoryPool */
void*           memoryAllocatorRelease(MemoryAllocator* allocator);

void*           memoryAllocatorAllocate(MemoryAllocator* allocator,size_t size);

/* Merge the next adjacent block is free */
void            memoryAllocatorFree(MemoryAllocator* allocator, void* ptr);

/* Merges all adjacent free blocks, and returns the size of largest free block */
size_t          memoryAllocatorOptimize(MemoryAllocator* allocator);

#endif /*/MEMORYALLOCATOR_MEMORYALLOCATOR_H */
