/**
 * @file MemoryManager.cpp
 *
 * @author Jason Slaughter, wintrywolf@gmail.com 
 * @date Oct 6, 2012
 */

#include "MemoryManager.h"

// TODO: Out of memory check.
MemoryManager::MemoryManager()
{
#ifdef E_DEBUG
    	mf_memManagerCore = fopen("MemoryManagerCoreDump.txt", "w");
#endif
    	m_ptrSize = sizeof(size_t);
    	m_pool = (ptrdiff_t*)malloc(sizeof(ptrdiff_t) * POOL_SIZE);
    	memset(m_pool, 0, POOL_SIZE * m_ptrSize);
    	m_pool[0] = -1 * (POOL_SIZE);
    	m_blocksAllocated = 0;
    	m_objectCount = 0;
    	m_loadValue = LOAD_FACTOR * POOL_SIZE;
    	m_loadReached = false;
    	m_deallocTable.insert(std::make_pair(m_pool, POOL_SIZE));
    	m_size = 1;
}

ptrdiff_t* MemoryManager::getBlock(size_t size)
{
	std::map<ptrdiff_t*, ptrdiff_t, std::less<ptrdiff_t*> >::iterator index = m_deallocTable.begin();
	while(size > index->second) {
		index++;
	}

	if(index->second == size) {
		ptrdiff_t* offset = index->first;
		m_deallocTable.erase(index);
		m_size--;
		return offset;
	}

	else if(m_size == m_deallocTable.size()) {  // If this is the rightmost free block (which implies no more fragments after this block).
		ptrdiff_t* offset = index->first;
		MemRecord mr(index->first + size, index->second - size);
		m_deallocTable.erase(index);
		m_deallocTable.insert(mr);
		return offset;
	}

	else if(size < index->second) {
		ptrdiff_t diff = index->second - size;
		ptrdiff_t* offset = index->first;
		ptrdiff_t* newoffset = index->first + size; // New offset to be inserted.
		MemRecord temp(newoffset,diff);
		m_deallocTable.erase(index);
		m_deallocTable.insert(temp);
		m_size++;
		return offset;
	}

	return 0;
}

std::pair<ptrdiff_t*, ptrdiff_t> MemoryManager::deallocUpdate(MemRecord mem)
{
	std::map<ptrdiff_t*, ptrdiff_t, std::less<ptrdiff_t*> >::iterator index = m_deallocTable.begin();

	if(m_deallocTable.size() == 1) {
		if(mem.first + mem.second == index->first) {
			MemRecord mr(index->first - mem.second, index->second + mem.second);
			m_deallocTable.erase(index);
			m_deallocTable.insert(mr);
			return mem;
		} else {
			m_deallocTable.insert(mem);
			m_size++;
			return mem;
		}
	}

	if(mem.first < index->first) { // If deallocated offset is smaller than any offset in the table.
		if(mem.first + mem.second == index->first) { // If the block is adjacent to the first free block.
			m_deallocTable.erase(index);
			m_deallocTable.insert(mem);
			return mem;
		} else {
			m_deallocTable.insert(mem);
			m_size++;
			return mem;
		}
	}

	index = m_deallocTable.upper_bound(mem.first);

	bool leftAdjacent = (index->first + index->second == mem.first); /* True if table entry before
	shows we have an empty block adjacent to our newly deallocated one on the left. */

	index++;

	bool rightAdjacent = (mem.first + mem.second == index->first);

	std::map<ptrdiff_t*, ptrdiff_t, std::less<ptrdiff_t*> >::iterator rightIter = index;
	index--;
	std::map<ptrdiff_t*, ptrdiff_t, std::less<ptrdiff_t*> >::iterator leftIter = index;

	if(leftAdjacent && rightAdjacent) {
		ptrdiff_t total = mem.second + leftIter->second + rightIter->second;
		m_deallocTable[leftIter->first] = total;
		m_deallocTable.erase(rightIter);
		m_size--;
		return *leftIter;
	}

	if(leftAdjacent && (!rightAdjacent)) {
		m_deallocTable[leftIter->first] += mem.second;
		return *leftIter;

	}

	if(rightAdjacent && (!leftAdjacent)) {
		ptrdiff_t temp = rightIter->second + mem.second;
		m_deallocTable.erase(rightIter);
		MemRecord mr(mem.first, temp);
		m_deallocTable.insert(mr);
		return mr;
	}

	m_deallocTable.insert(mem);
	m_size++;
	return mem;
}

ptrdiff_t* MemoryManager::getExact(size_t size)
{
	return 0;
}

#ifdef E_DEBUG
	int MemoryManager::getObjectCount()
	{
		return MemoryManager::get().m_objectCount;
	}

	ptrdiff_t MemoryManager::whereAmI(void* object)
	{
		ptrdiff_t* o = (ptrdiff_t*)object;
		if((o < MemoryManager::get().m_pool) || (o > (MemoryManager::get().m_pool + POOL_SIZE - 1))) {
			return -1;
		}

		return ((o - MemoryManager::get().m_pool) / MemoryManager::get().m_ptrSize);
	}

	void MemoryManager::dumpCore()
	{
		fwrite(MemoryManager::get().m_pool, MemoryManager::get().m_ptrSize, POOL_SIZE, MemoryManager::get().mf_memManagerCore);
	}
#endif

MemoryManager& MemoryManager::get()
{
	static MemoryManager instance;
	return instance;
}

void* MemoryManager::allocate(size_t size)
{
	int realSize = (std::ceil((float)size / m_ptrSize) + 1); // Size for memory and the header.  Must be some multiple of pointer size
	ptrdiff_t* offset = getBlock(realSize);
	*offset = realSize;
	m_objectCount++;
	m_blocksAllocated += realSize;
	return offset + 1;
}

void* MemoryManager::managedMalloc(size_t size)
{
	return MemoryManager::get().allocate(size);
}

void* MemoryManager::managedRealloc(void* ptr, size_t size)
{
	if(ptr == NULL) {
		return MemoryManager::get().allocate(size);
	}

	// TODO: If ptr is not NULL and size is 0.

	ptrdiff_t* block = (ptrdiff_t*)ptr;

	if(block < MemoryManager::get().m_pool || block >= ( MemoryManager::get().m_pool + (POOL_SIZE - 1) )) { // If user tries to pass in unmanaged memory
		return block;
	}

	ptrdiff_t meta = block[-1];
	ptrdiff_t needed = size + 1;

	if((meta) == needed) {
		return block;
	}

	if(meta > needed) {
		block[needed - 1] = -1 * (meta - needed);
		MemoryManager::get().deallocate(&block[needed - 1]);
		return block;
	}

	if(meta < needed) {

		if(block[meta - 1] < -1 && (-1 * block[meta - 1] + meta >= needed + 1)) { /* If block next to this is a free one and can
			accommodate what is needed, plus one to avoid using the whole free block -- which would necessitate removing that record --
			and thus modify the record instead which saves us from having to do another check to see if whole block is used. */
			ptrdiff_t diff = needed - meta;

			std::map<ptrdiff_t*, ptrdiff_t, std::less<ptrdiff_t*> >::iterator index;
			index = MemoryManager::get().m_deallocTable.find(&block[meta - 1]);
			MemRecord newRecord(index->first + diff, index->second - diff);
			MemoryManager::get().m_deallocTable.erase(index);
			MemoryManager::get().m_deallocTable.insert(newRecord);

			return block;
		}
		else {
			ptrdiff_t* source = block;
			ptrdiff_t* dest = (ptrdiff_t*)MemoryManager::get().allocate(needed);
			std::copy(source, source + meta - 1, dest); // Copy contents as required by realloc in ISO C.
			MemoryManager::get().deallocate(source);
			return dest;
		}
	}
	return (void*)0; // To silence warnings.  Function should never reach here.
}

void MemoryManager::managedFree(void* ptr)
{
	// TODO: Error on attempt to free unmanaged memory.
	MemoryManager::get().deallocate(ptr);
}

void MemoryManager::deallocate(void* mem)
{
	ptrdiff_t* offset = (ptrdiff_t*)mem - 1;
	MemRecord mr(offset, *offset);
	MemRecord temp = deallocUpdate(mr);
	*offset = -1 * temp.second;
	m_objectCount--;
	m_blocksAllocated -= temp.second;
}

MemoryManager::~MemoryManager()
{
	free(m_pool);
	m_pool = nullptr;
}

void* operator new(size_t size) throw(std::bad_alloc)
{
	return MemoryManager::get().allocate(size);
}

void* operator new(size_t size, const std::nothrow_t& nothrow_constant) throw()
{
	return MemoryManager::get().allocate(size);
}


void operator delete(void* p) throw()
{
	MemoryManager::get().deallocate(p);
}

void operator delete (void* p, const std::nothrow_t& nothrow_constant) throw()
{
	MemoryManager::get().deallocate(p);
}

void* operator new[](size_t size) throw(std::bad_alloc)
{
	return MemoryManager::get().allocate(size);
}

void* operator new[](size_t size, const std::nothrow_t& nothrow_constant) throw()
{
	return MemoryManager::get().allocate(size);
}

void operator delete[](void* p) throw()
{
	MemoryManager::get().deallocate(p);
}

void operator delete[] (void* p, const std::nothrow_t& nothrow_constant) throw()
{
	MemoryManager::get().deallocate(p);
}
