/*
 * @Author: yyx-pc 3454523412@qq.com
 * @Date: 2024-11-08 15:43:24
 * @LastEditors: yyx-pc 3454523412@qq.com
 * @LastEditTime: 2024-11-08 16:06:09
 * @FilePath: \yxstl\include\allocator.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE`
 */
#include <new>

namespace yxstl{
    class allocator{
        allocator::value_type;
        allocator::pointer;
        allocator::const_pointer;
        allocator::reference;
        allocator::const_reference;
        allocator::size_type;
        allocator::difference_type;

        // 一个嵌套的class template，class rebind<U> 拥有唯一成员other,是一个typedef，代表allocator<U>
        allocator::rebind;

        // 构造函数
        allocator::allocator()
            // 拷贝构造函数
            allocator::allocator(const allocator &) template <class U>
            allocator::allocator(const allocator<U> &)
            // 析构函数
            allocator::~allocator();

        // 返回某个对象的地址，等同于&x
        pointer allocator::address(reference x) const
            const_pointer allocator::address(const_reference x) const

            // 分配空间，足以容纳n个元素
            pointer allocator::allocate(size_type n, const void * = 0)
            // 归还之前分配的空间
            void allocator::deallocate(pointer p, size_type n)
            // 可分配的最大空间
            size_type allocator::max_size() const

            // 通过x，在p指向的地址构造一个对象。相当于new((void*)p) T(x)
            void allocator::construct(pointer p, const T &x)
            // 析构地址p的对象
            void allocator::destroy(pointer p)
    };
}