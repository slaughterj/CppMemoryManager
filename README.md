CppMemoryManager
================

A fragment coalescing memory manager for C++ projects. 

###Description

When included in a C++ project, this essentially replaces the default new and delete operators with the implementation
provided by the memory manager. A large block of memory, whose size can be changed in the source, is allocated from the
operating system upon program start. All calls to new and delete are handled by the memory manager using this memory block,
bypassing the OS and thus likely improving performance compared to the default allocator. The manager automatically coalesces
empty contiguous blocks of freed memory upon calls to delete, keeping fragmentation to a minimum. It can also use a best
fit allocation strategy when fragmentation pressure is high.

###Usage

```C++

#include <iostream>
#include "MemoryManager.h"

int main()
{
    int* numArray = new int[100];
    delete [] numArray;
    
    return 0;
}
```

It's that simple. When the memory manager is linked in or the header file is included anywhere in your C++ source, the
default OS-provided new and delete implementations are replaced by the one from this.
