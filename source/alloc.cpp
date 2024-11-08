/*
 * @Author: yyx-pc 3454523412@qq.com
 * @Date: 2024-11-08 15:40:13
 * @LastEditors: yyx-pc 3454523412@qq.com
 * @LastEditTime: 2024-11-08 18:29:28
 * @FilePath: \code\yxstl\source\alloc.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "include/alloc.h"

namespace yxstl {

char *alloc::start_free = 0;
char *alloc::end_free = 0;
size_t alloc::heap_size = 0;
alloc::obj *alloc::free_list[alloc::__NFREELISTS] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

void *alloc::allocate(size_t n) {
    if (n > (size_t)__MAX_BYTES)
        return malloc(n);
    alloc::obj **my_free_list;
    alloc::obj *res;
    my_free_list = alloc::free_list + alloc::FREELIST_INDEX(n);
    res = *my_free_list;
    if (res == 0) {
        // 没找到可用的free-list，准备重新填充free-list
        void *r = refill(ROUND_UP(n));
        return r;
    }
    // 调整free list
    *my_free_list = res->free_list_link;
    return (res);
}

void alloc::deallocate(void *ptr, size_t bytes) {
    if (bytes > (size_t)__MAX_BYTES)
        free(ptr);
    else {
        alloc::obj **my_free_list;
        alloc::obj *res = (alloc::obj *)ptr;
        // 寻找对应的free-list
        my_free_list = alloc::free_list + alloc::FREELIST_INDEX(bytes);
        // 调整free-list，回收区块
        res->free_list_link = *my_free_list;
        *my_free_list = res;
    }
}

void *alloc::reallocate(void *ptr, size_t old_sz, size_t new_sz) {
    deallocate(ptr, old_sz);
    ptr = allocate(new_sz);
    return ptr;
}

void *alloc::refill(size_t n) {
    int nobjs = 20;
    // 调用chunk_alloc()，尝试取得njobs个区块作为free-list的新节点
    // 参数nobjs是pass by reference
    char *chunk = chunk_alloc(n, nobjs);
    alloc::obj **my_free_list;
    alloc::obj *result;
    alloc::obj *current_obj, *next_obj;
    int i;

    // 如果只获得一个区块，这个区块就分配给调用者用，free-list无新节点
    if (1 == nobjs)
        return (chunk);
    // 否则准备调整free-list，纳入新节点
    my_free_list = free_list + FREELIST_INDEX(n);

    /* 在chunk空间内建立free-list */
    result = (obj *)chunk;  // 这一块准备返回给客户
    // 以下引导free-list指向新分配的空间（取自内存池）
    *my_free_list = next_obj = (obj *)(chunk + n);
    // 以下将free-list的各节点串接起来
    for (i = 1;; i++) {  // 从1开始，因为第0个将返回给客户
        current_obj = next_obj;
        next_obj = (obj *)((char *)next_obj + n);
        if (nobjs - 1 == i) {
            current_obj->free_list_link = 0;
            break;
        } else {
            current_obj->free_list_link = next_obj;
        }
    }
    return (result);
}

char *alloc::chunk_alloc(size_t size, int &nobjs) {
    char *result;
    size_t total_bytes = size * nobjs;
    size_t bytes_left = end_free - start_free;  // 内存池剩余空间

    if (bytes_left >= total_bytes) {
        // 内存池剩余空间完全满足需求量
        result = start_free;
        start_free += total_bytes;
        return (result);
    } else if (bytes_left >= size) {
        // 内存池剩余空间不能完全满足需求量，但足够供应一个(含)以上的区块
        // 那么能分配多少区块，就分配多少区块
        nobjs = bytes_left / size;   // 可分配的区块数
        total_bytes = size * nobjs;  // 总共可分配的字节数
        result = start_free;
        start_free += total_bytes;
        return (result);
    } else {  // 内存池剩余空间连一个区块的大小都无法提供
        // 计算需要从heap申请的空间数量，用以补充内存池
        size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
        // 试着让内存池中的残余零头还有利用价值
        if (bytes_left > 0) {
            // 内存池中还有一些零头，先配给适当的free-list
            // 首先寻找适当的free-list
            obj **my_free_list = free_list + FREELIST_INDEX(bytes_left);
            // 调整free-list，将内存池中的残余空间编入
            ((obj *)start_free)->free_list_link = *my_free_list;
            *my_free_list = (obj *)start_free;
        }
        // 调用malloc，分配heap空间，用来补充内存池
        start_free = (char *)malloc(bytes_to_get);
        if (0 == start_free) {
            // heap空间不足，malloc()失败
            int i;
            obj **my_free_list, *p;
            // 试着检视我们手上拥有的东西。这不会造成伤害。我们不打算尝试分配
            // 较小的区块，因为那在多进程机器上容易导致灾难
            // 以下搜寻适当的free-list
            // 所谓适当是指”尚有未用区块，且区块够大“的free-list
            for (i = size; i <= __MAX_BYTES; i += __ALIGN) {
                my_free_list = free_list + FREELIST_INDEX(i);
                p = *my_free_list;
                if (0 != p) {  // free-list内尚有未用区块
                    // 调整free-list以释出未用区块
                    *my_free_list = p->free_list_link;
                    start_free = (char *)p;
                    end_free = start_free + i;
                    // 递归调用自身，为了修正nobjs
                    return (chunk_alloc(size, nobjs));
                    // 任何残余零头终将被编入适当的free-list中备用
                }
            }
            end_free = 0;  // 山穷水尽，到处都没有内存可用
            // 调用第一级分配器，看看out-of-memory机制能否尽点力
            start_free = (char *)malloc(bytes_to_get);
            // 可能会抛出异常，或者内存不足的情况获得改善
        }
        heap_size += bytes_to_get;
        end_free = start_free + bytes_to_get;
        // 递归调用自身，为了修正nobjs
        return (chunk_alloc(size, nobjs));
    }
}

}  // namespace yxstl