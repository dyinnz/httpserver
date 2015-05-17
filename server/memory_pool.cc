#include "memory_pool.h"

/******************************************************************************/
/* Construction. Allocate a block of memory */

MemoryPool::MemoryPool(size_t size) : large_(NULL) {
    data_ = malloc_wrap<BlockData>(size);

    data_->last = (char*)data_ + sizeof(BlockData);
    data_->end = (char*)data_ + size;
    data_->next = NULL;
    data_->failed = 0;

    first_ = data_;

    max_alloc_ = kMaxAlloc > size - sizeof(BlockData) ? kMaxAlloc : size - sizeof(BlockData);
}

/******************************************************************************/
/* Destruction. Free block memory and large memory */

MemoryPool::~MemoryPool() {
    BlockData *b = first_;
    while (b) {
        BlockData *temp = b;
        b = b->next;
        free(temp);
    }

    LargeMemory *l = large_;
    while (l) {
        LargeMemory *temp = l;
        l = l->next;
        free(temp);
    }
}

/******************************************************************************/

void* MemoryPool::AllocateLarge(size_t size) {
    LargeMemory *p = malloc_wrap<LargeMemory>(size + sizeof(LargeMemory));

    p->next = large_;
    p->alloc = (char*)p + sizeof(LargeMemory);
    large_  = p;

    return p->alloc;
}

void* MemoryPool::AllocateBlock(size_t size) {
    size_t block_size = (size_t)(data_->end - (char*)data_);
    BlockData *block = malloc_wrap<BlockData>(size);

    block->end = (char*)block + block_size;
    block->next = NULL;
    block->failed = 0;

    data_->last = (char*)data_ + sizeof(BlockData) + size;

    BlockData *b = data_;
    while (b->next) {
        if (b->failed++ > 4) {
            data_ = b->next;
        }
        b = b->next;
    }

    b->next = block;

    return data_->last;
}

void* MemoryPool::Allocate(size_t size) {
    if (size <= max_alloc_) {

        for (BlockData *p = data_; p; p = p->next) {

            if ((size_t)(p->end - p->last) >= size) {
                void *ret = p->last;
                p->last += size;
                return ret;
            }
        }

        return AllocateBlock(size);

    } else {
        return AllocateLarge(size);
    }
}

template <class T> 
T* MemoryPool::Allocate(size_t n) {
    return (T*)Allocate(sizeof(T) * n);
}

/* Only free large memory */

void MemoryPool::Free(void *p) {
    if (!p) return;

    if (p == large_) {
        large_ = large_->next;
        free(p);
    }

    LargeMemory *l = large_;
    while (l) {
        if (l->next == p) {
            l->next = l->next->next;
            free(p);
            break;
        }
        l = l->next;
    }
}

void TestMemoryPool() {
    MemoryPool mem_pool(1024);
    auto *p = mem_pool.Allocate<int>();
    *p = 10;
    printf("%x, %d\n", (unsigned int)p, *p);
}

/******************************************************************************/
