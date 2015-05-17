/*
 * Author: DY.HUST
 * Date: 2015/05/17
 * Email: ml_143@sina.com
 */

#pragma once

#include <cstdlib>
#include "utility.h"

/******************************************************************************/
/* Memory pool. Release memory at last. */

class MemoryPool {
public:
    MemoryPool(size_t size);
    ~MemoryPool();

private:
    struct BlockData {
        char        *last;
        char        *end;
        BlockData    *next;
        size_t      failed;
    };

    struct LargeMemory {
        LargeMemory *next;
        char        *alloc;
    };

    static const size_t kMaxAlloc = 4096;

    size_t      max_alloc_;
    LargeMemory *large_;
    BlockData    *data_;
    BlockData    *first_;

    void* AllocateBlock(size_t size);
    void* AllocateLarge(size_t size);

public:
    void* Allocate(size_t size);
    template <class T> T* Allocate(size_t n = 1);

    void Free(void *p);
};

/******************************************************************************/
/* Memory allocate wrap function */
template <class T> inline
T *malloc_wrap(size_t size) {
    T *p = (T*)malloc(size);
    if (!p) {
        http_log(kEmergency, "Out of memory, exit.\n");
        exit(-1);
    }
    return p;
}

void TestMemoryPool();

