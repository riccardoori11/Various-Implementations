#include <charconv>
#include <assert.h>
#include <deque_fixed_size.hpp>
#include <iostream>

int main(){

		ricc::Dequeue_fixed_size<int> a;

		a.pushBack(4);

		assert(a.size() == 1);

		assert(a.at(0) == 4);

}

