/**
 * @file MemoryManager.h
 *
 * @author Jason Slaughter, wintrywolf@gmail.com
 * @date 1 December 2012
 * @brief A simple, general purpose memory manager.
 *
 * @details
 */

#ifndef MEMORYMANAGER_H_
#define MEMORYMANAGER_H_

#include <cstring>
#include <cstdlib>
#include <memory>
#include <algorithm>
#include <utility>
#include <cmath>
#include <new>
#include <map>
#include "NodeAllocator.h"

static const int MEGABYTE = 1048576;
static const int POOL_SIZE = (10 * MEGABYTE) / sizeof(size_t);
static const int RECORD_SIZE = 10000;
static const float LOAD_FACTOR = 0.80; // Used to determine when to switch allocation algorithm. Must be 0.99 or lower.

typedef unsigned char byte_t;
typedef std::pair<ptrdiff_t*, ptrdiff_t> MemRecord;
typedef std::map<ptrdiff_t*, ptrdiff_t, std::less<ptrdiff_t*>, NodeAllocator<MemRecord, RECORD_SIZE> > MemMap;

class MemoryManager
{
	// Global new and delete operators including array versions replaced.
	friend void* ::operator new(size_t size) throw(std::bad_alloc);
	friend void* ::operator new(size_t size, const std::nothrow_t& nothrow_constant) throw();
	friend void ::operator delete(void* p) throw();
	friend void ::operator delete(void* p, const std::nothrow_t& nothrow_constant) throw();
	friend void* ::operator new[](size_t size) throw(std::bad_alloc);
	friend void* ::operator new[](size_t size, const std::nothrow_t& nothrow_constant) throw();
	friend void ::operator delete[](void* p) throw();
	friend void ::operator delete[] (void* p, const std::nothrow_t& nothrow_constant) throw();

private:

#ifdef E_DEBUG
	FILE* mf_memManagerCore;
#endif

	ptrdiff_t* m_pool; // The raw data pool.  With blocks the size of pointers, 4 bytes on x86 and 8 bytes on x86_64.
    int m_objectCount; // The number of objects allocated, user defined and POD. Note that arrays are considered a single object.
    size_t m_blocksAllocated; // The number of blocks allocated.
    size_t m_ptrSize; // The size in bytes of a pointer on the current architecture.
    size_t m_loadValue; // LOAD_FACTOR * POOL_SIZE
    bool m_loadReached; // True if m_loadValue >= m_blocksAllocated.  Causes allocation to use different algorithm.
    MemMap m_deallocTable;
    int m_size;


private:
    MemoryManager(const MemoryManager& other); // Disallow copy constructor
    MemoryManager& operator=(const MemoryManager& other); // Disallow copy assignment

    inline ptrdiff_t* getBlock(size_t size);
    inline ptrdiff_t* getExact(size_t size);
    inline MemRecord deallocUpdate(MemRecord mem);
    inline static MemoryManager& get();

private:
    void* allocate(size_t size);
    void deallocate(void* p);

public:

#ifdef E_DEBUG
    static int getObjectCount();
    static ptrdiff_t whereAmI(void* object);
    static void dumpCore();
#endif

    MemoryManager();

    static void* managedMalloc(size_t size);
    static void* managedRealloc(void* ptr, size_t size);
    static void managedFree(void* ptr);


    ~MemoryManager();

};

#endif // MEMORYMANAGER_H_
