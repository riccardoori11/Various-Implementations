Going to be a directory where i try to implement various c++ object and data structures from the standard library


Small String Optimization:
broadly speaking, small string optimization is the practice in which you seperate 2 distinct cases. The first case is when the string is smaller than N bytes, in which you store the string directly on to the object whereas the second case is when >= N bytes, in which you would store it on the heap. This is primarily due to the fact that an inline buffer comes with the object and allocating to it is often cheaper than allocating on the heap since allocating on the heap comes with a variety of differnet memory overhead such as searching for a free block, storing metadata, etc...
In addition to that, using a pointer to follow the character may bring additional performance overhead due to certain cache/locality cost


Debugging with gdb
g++ -std=c++20 -g -O0 -Wall -Wextra -Wpedantic \
    -fsanitize=address,undefined main.cpp -o unique_ptr


