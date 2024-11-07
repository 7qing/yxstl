#ifndef ALLOC_H
#define ALLOC_H

#include <cstdlib>

namespace yxstl
{
    class alloc
    {
        enum
        {
            __ALIGN = 8
        }; // 小型区块的上调边界
        enum
        {
            __MAX_BYTES = 128
        }; // 小型区块的上限
        enum
        {
            __NFREELISTS = __MAX_BYTES / __ALIGN
        }; // free-list的个数
        // 使用union解决free-list带来的额外负担：维护链表所必须的指针而造成内存的另一种浪费
        union obj
        {
            union obj *free_list_link; // 系统视角
            char client_data[1];       // 用户视角
        };
        static size_t ROUND_UP(size_t bytes)
        {
            return (((bytes) + __ALIGN - 1) & ~(__ALIGN - 1));
        }
        // 16个free-list
        static obj *volatile free_list[__NFREELISTS];
        // 根据区块大小，决定使用第n号free-list。n从0算起
        static size_t FREELIST_INDEX(size_t bytes)
        {
            return (((bytes) + __ALIGN - 1) / __ALIGN - 1);
        }

        // 返回一个大小为n的对象，并可能加入大小为n的其它区块到free-list
        static void *refill(size_t n);
        // 分配一大块空间，可容纳nobjs个大小为”size“的区块
        // 如果分配nobjs个区块有所不便，nobjs可能会降低
        static char *chunk_alloc(size_t size, int &nobjs);

        // Chunk allocation state.
        static char *start_free; // 内存池起始位置。只在chunk_alloc()中变化
        static char *end_free;   // 内存池结束位置。只在chunk_alloc()中变化
        static size_t heap_size;

    public:
        static void *allocate(size_t n);
        static void deallocate(void *p, size_t n);
        static void *reallocate(void *p, size_t old_sz, size_t new_sz);
    };
}

#endif