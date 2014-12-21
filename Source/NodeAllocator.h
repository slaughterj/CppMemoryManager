/**
 * @file NodeAllocator.h
 *
 * @author Jason Slaughter, wintrywolf@gmail.com 
 * @date Feb 2, 2013
 */

#ifndef NODEALLOCATOR_H_
#define NODEALLOCATOR_H_

/* The following code example is taken from the book
 * "The C++ Standard Library - A Tutorial and Reference"
 * by Nicolai M. Josuttis, Addison-Wesley, 1999
 *
 * (C) Copyright Nicolai M. Josuttis 1999.
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 */

/* Modified by Jason Slaughter slaughter@wayne.edu */

#include <limits>
#include <cstdlib>
#include <cmath>

namespace
{

template <typename Type, int number>
class NodeAllocatorPool
{
private:
    typedef unsigned char byte_t;
    byte_t** m_stack;
    byte_t* m_pool;
    int m_index;

public:
    enum
    {
        capacity = number,
        objectSize = sizeof(Type)
    };

    NodeAllocatorPool() : m_index(capacity - 1), m_pool((byte_t*)malloc(objectSize * number)),
    		m_stack((byte_t**)malloc(sizeof(byte_t*) * capacity))
    {
    	byte_t* temp = m_pool;
        for(int i = 0; i < capacity; i++) {
            m_stack[m_index] =  temp;
            temp += objectSize;
            m_index--;
        }
        m_index = capacity - 1;
    }

    void* allocate(size_t)
    {
        byte_t* mem = m_stack[m_index];
        m_index--;

        return mem;
    }

    void deallocate(void* p)
    {
    	m_index++;
        m_stack[m_index] = (byte_t*)p;
    }

    ~NodeAllocatorPool()
    {
    	free(m_pool);
    }
};

} // End unnamed namespace.

   template <class T, int reserve>
   class NodeAllocator {
   private:
       NodeAllocatorPool<T, reserve> pool;

     public:
       // type definitions
       typedef T        value_type;
       typedef T*       pointer;
       typedef const T* const_pointer;
       typedef T&       reference;
       typedef const T& const_reference;
       typedef std::size_t    size_type;
       typedef std::ptrdiff_t difference_type;

       // rebind allocator to type U
       template <class U>
       struct rebind {
           typedef NodeAllocator<U, reserve> other;
       };

       // return address of values
       pointer address (reference value) const {
           return &value;
       }
       const_pointer address (const_reference value) const {
           return &value;
       }

       /* constructors and destructor
        * - nothing to do because the allocator has no state
        */
       NodeAllocator() throw() {

       }
       NodeAllocator(const NodeAllocator&) throw() {

       }
       template <class U>
         NodeAllocator (const NodeAllocator<U, reserve>&) throw() {

       }
       ~NodeAllocator() throw() {
       }

       // return maximum number of elements that can be allocated
       size_type max_size () const throw() {
           return std::numeric_limits<std::size_t>::max() / sizeof(T);
       }

       // allocate but don't initialize num elements of type T
       pointer allocate (size_type num, const void* = 0) {
           pointer ret = (pointer)(pool.allocate(num*sizeof(T)));
           //printf("Allocated %ld of size %ld \n", num, sizeof(T));
           return ret;
       }

       // initialize elements of allocated storage p with value value
       void construct (pointer p, const T& value) {
           // initialize memory with placement new
           new((void*)p)T(value);
       }

       // destroy elements of initialized storage p
       void destroy (pointer p) {
           // destroy objects by calling their destructor
           p->~T();
       }

       // deallocate storage p of deleted elements
       void deallocate (pointer p, size_type num) {
           pool.deallocate((void*)p);
          // printf("Deallocated %ld \n", num, sizeof(T));
       }
   };

   // return that all specializations of this allocator are interchangeable
   template <class T1, class T2>
   bool operator== (const NodeAllocator<T1, 0>&,
                    const NodeAllocator<T2, 0>&) throw() {
       return true;
   }
   template <class T1, class T2>
   bool operator!= (const NodeAllocator<T1, 0>&,
                    const NodeAllocator<T2, 0>&) throw() {
       return false;
   }

#endif /* NODEALLOCATOR_H_ */
