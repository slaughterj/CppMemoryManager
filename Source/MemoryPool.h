/**
 * @file MemoryPool.h
 *
 * @author Jason Slaughter, wintrywolf@gmail.com 
 * @date Feb 9, 2013
 */

#ifndef MEMORYPOOL_H_
#define MEMORYPOOL_H_

template <typename Type, int number>
class MemoryPool
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

    MemoryPool() : m_index(capacity - 1), m_pool(new Type[capacity]),
    		m_stack(new byte_t*[capacity])
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

    ~MemoryPool()
    {
    	delete [] m_pool;
    	delete [] m_stack;
    }
};

#define DECLARE_POOLED_CLASS(class, number) \
    private: \
        static Pool<class,number> class##Pool; \
    public: \
        void* operator new(size_t size); \
        void operator delete(void* p); \
    private:

#define DEFINE_POOLED_CLASS(class, number) \
    Pool<class,number> class::class##Pool; \
    void* operator new(size_t size) \
    { \
        return class##Pool.allocate(size); \
    } \
    void operator delete(void* p) \
    { \
        class##Pool.deallocate(p); \
    }


#endif /* MEMORYPOOL_H_ */
