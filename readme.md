Going to be a directory where i try to implement various c++ object and data structures from the standard library


Small String Optimization:
broadly speaking, small string optimization is the practice in which you seperate 2 distinct cases. The first case is when the string is smaller than N bytes, in which you store the string directly on to the object whereas the second case is when >= N bytes, in which you would store it on the heap. This is primarily due to the fact that an inline buffer comes with the object and allocating to it is often cheaper than allocating on the heap since allocating on the heap comes with a variety of differnet memory overhead such as searching for a free block, storing metadata, etc...
In addition to that, using a pointer to follow the character may bring additional performance overhead due to certain cache/locality cost


Debugging with gdb
g++ -std=c++20 -g -O0 -Wall -Wextra -Wpedantic \
    -fsanitize=address,undefined main.cpp -o unique_ptr


Dequeue
As opposed to std::vector, the elements of a deque are not stored contiguously: typical implementations use a sequence of individually allocated fixed-size arrays, with additional bookkeeping, which means indexed access to deque must perform two pointer dereferences, compared to vector's indexed access which performs only one.

Has higher minimal memory cost than a vector 

https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p0447r28.html?utm_source=chatgpt.com#introduction


The three core aspects of a hive from an abstract perspective are:

    A collection of element blocks + metadata, to prevent reallocation during insertion (as opposed to a single element block).
    A method of skipping erased elements in O(1) time during iteration (as opposed to reallocating subsequent elements during erasure).
    An erased-element location recording mechanism, to enable the re-use of memory from erased elements in subsequent insertions, which in turn increases cache locality and reduces the number of block allocations/deallocations.
https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p0447r28.html?utm_source=chatgpt.com
