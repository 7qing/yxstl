#include "include/alloc.h"

namespace yxstl
{
    char *alloc::start_free = 0;
    char *alloc::end_free = 0;
    size_t alloc::heap_size = 0;
    alloc::obj *volatile alloc::free_list[alloc::__NFREELISTS] = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    void *alloc::allocate(size_t n)
    {
        if (n > (size_t)__MAX_BYTES)
            return malloc(n);
        alloc::obj *volatile *my_free_list;
        alloc::obj *volatile res;
        res = *(alloc::free_list + alloc::FREELIST_INDEX(n));
        if (res)
        { // 此list还有空间给我们
            free_list[res] = res->free_list_link;
            return res;
        }
    }
}