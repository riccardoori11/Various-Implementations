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

		{

				ricc::Dequeue_fixed_size<ricc::Tracker> b;
				b.push_back(ricc::Tracker());
				assert(ricc::Tracker::alive == 1);

		}
		assert(ricc::Tracker::alive == 0);
		assert(ricc::Tracker::destroyed == ricc::Tracker::constructed);

		ricc::Dequeue_fixed_size<int> c(a);
		assert(a.size() == 2);
		assert(a.at(0) == 1);
		assert(a.at(1) == 4);
		ricc::Dequeue_fixed_size<int> d(std::move(c));
		assert(d.size() == 2);
		assert(d.at(0) == 1);
		assert(d.at(1) == 4);
		

}

