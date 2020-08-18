#include <assert.h>
#include "memoryAllocator.h"
#define ALIGNMENT 8

struct MemoryAllocator
{
    size_t   m_size;
    void     *m_memoryPool;
};

/*---------- Auxiliary Functions --------------*/

static size_t getSizeWithAlignment(size_t size)
{
    return (((size + ALIGNMENT - 1) / ALIGNMENT) * ALIGNMENT) + sizeof(size_t);
}

static void * getEndOfMemoryPool(MemoryAllocator *allocator)
{
    return (char *)(allocator -> m_memoryPool) + (allocator -> m_size);
}

static char isFreeBlock(void *block)
{
    return block ? !(*(size_t *)block & 1) : -1;
}

static size_t getBlockSize(void *block)
{
    return block ? *(size_t *)(block) & ~(size_t)1 : 0;
}

static void * getNextBlock(void * block, void * endOfBlock)
{
    return  !block || ((char *)block + getBlockSize(block)) >= (char *)endOfBlock ? NULL :  (char *)block + getBlockSize(block);
}

static void signBlockAsOccupied(void *block)
{
    *(size_t *)block |= 1;
}

static void signBlockAsFree(void * block)
{
    *(size_t *)block = getBlockSize(block);
}

static void * getFirstFreeBlock(void *block, void *endOfMemoryPool)
{
    if(isFreeBlock(block))
    {
        return block;
    }

    return getFirstFreeBlock(getNextBlock(block, endOfMemoryPool), endOfMemoryPool);
}

static void splitBlock(void * block, size_t size)
{
    *(size_t *)((char *)block + size) = *(size_t *)block - size;
    *(size_t *)block = size;
    signBlockAsOccupied(block);
}

static void mergeBlocks(size_t *blockA, size_t *blockB)
{
    assert(blockA);
    assert(blockB);

    *(size_t *)blockA = getBlockSize(blockA) + getBlockSize(blockB);
}

static void mergeSumFreeSequenceBlocks(void *block, void *endOfMemoryPool, size_t size)
{
    void *nextBlock = getNextBlock(block, endOfMemoryPool);

    while(isFreeBlock(nextBlock) > 0 && getBlockSize(block) < size)
    {
        mergeBlocks(block, nextBlock);
        nextBlock = getNextBlock(block, endOfMemoryPool);
    }
}

static void * prepareRelevantBlock(MemoryAllocator* allocator, size_t size)
{
    void *freeBlock = allocator -> m_memoryPool;

    while(1)
    {
        freeBlock = getFirstFreeBlock(freeBlock, getEndOfMemoryPool(allocator));
        if(!freeBlock)
        {
            break;
        }

        if(getBlockSize(freeBlock) < size)
        {
            mergeSumFreeSequenceBlocks(freeBlock, getEndOfMemoryPool(allocator), size);
        }

        if(getBlockSize(freeBlock) == size)
        {
            signBlockAsOccupied(freeBlock);
            break;
        }

        if(getBlockSize(freeBlock) > size)
        {
            splitBlock(freeBlock, size);
            break;
        }

        freeBlock = getNextBlock(freeBlock, getEndOfMemoryPool(allocator));
    }
    return freeBlock;
}

/*---------- API Functions ------------ */

MemoryAllocator* memoryAllocatorInit(void* memoryPool, size_t size)
{
    MemoryAllocator *memoryAllocator = malloc(sizeof(MemoryAllocator));

    assert(memoryPool);

    if(memoryAllocator)
    {
        memoryAllocator -> m_size = getSizeWithAlignment(size);
        memoryAllocator -> m_memoryPool = memoryPool;

        *(size_t *)(memoryAllocator -> m_memoryPool) = memoryAllocator -> m_size; /*TODO insertMataData func*/
    }

    return memoryAllocator;
}

void* memoryAllocatorRelease(MemoryAllocator* allocator)
{
    void * tmp = NULL;

    assert(allocator);

    if(allocator)
    {
        tmp = allocator -> m_memoryPool;

        free(allocator); 
    }
    
    return tmp;
}

void* memoryAllocatorAllocate(MemoryAllocator* allocator, size_t size)
{
    void *block;

    assert(allocator);
    
    if(!allocator)
    {
        return NULL;
    }

    /* not allow empty block */
    if(!size)
    {
        return NULL;
    }

    block = prepareRelevantBlock(allocator, getSizeWithAlignment(size));

    return block ? (size_t *)block + 1 : block;
}

void memoryAllocatorFree(MemoryAllocator* allocator, void *ptr)
{
    void *nextBlock;

    assert(allocator);

    if(ptr && allocator)
    {
        ptr = (size_t *)ptr - 1;
        signBlockAsFree(ptr);
        nextBlock = getNextBlock(ptr, getEndOfMemoryPool(allocator));

        if(nextBlock && isFreeBlock(nextBlock))
        {
            mergeBlocks(ptr, nextBlock);
        }
    }
}

size_t memoryAllocatorOptimize(MemoryAllocator *allocator)
{
    size_t maxSize = 0;
    void * block, *endOfMemoryPool;

    assert(allocator);

    if(!allocator)
    {
        return 0;
    }
    
    block =  allocator -> m_memoryPool;
    endOfMemoryPool  = getEndOfMemoryPool(allocator);

    while(block)
    {
        block = getFirstFreeBlock(block, endOfMemoryPool);

        mergeSumFreeSequenceBlocks(block, endOfMemoryPool, allocator -> m_size);

        maxSize = maxSize > getBlockSize(block) ? maxSize : getBlockSize(block);

        block = getNextBlock(block, endOfMemoryPool);
    }

    return maxSize;
}