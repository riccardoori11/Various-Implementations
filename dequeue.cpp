#include <charconv>
#include <assert.h>
#include <deque_fixed_size.hpp>
#include <iostream>

int main(){

		ricc::Dequeue_fixed_size<int> a;

		a.push_back(4);

		assert(a.size() == 1);

		assert(a.at(0) == 4);

		a.push_front(1);
		
		assert(a.at(0) == 1);
		assert(a.at(1) == 4);
		assert(a.size() == 2);

		ricc::Dequeue_fixed_size<int> c(a);
		assert(a.size() == 2);
		assert(a.at(0) == 1);
		assert(a.at(1) == 4);
		ricc::Dequeue_fixed_size<int> d(std::move(c));
		assert(d.size() == 2);
		assert(d.at(0) == 1);
		assert(d.at(1) == 4);
		
		c = d;
		
		assert(c.size() == 2);
		assert(c.at(0) == 1);
		assert(c.at(1) == 4);
		assert(d.size() == 2);
		assert(d.at(0) == 1);
		assert(d.at(1) == 4);

		ricc::Dequeue_fixed_size<int> e ;

		e = std::move(d);
		assert(e.size() == 2);
		assert(e.at(0) == 1);
		assert(e.at(1) == 4);

		e.popBack();
		e.push_back(2);

		assert(e.at(1) == 2);

		e.popFront();
		e.push_front(9);
		assert(e.at(0) == 9);
}

