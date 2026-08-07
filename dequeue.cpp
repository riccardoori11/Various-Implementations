#include "dequeue.hpp"
#include <charconv>
#include <iostream>

int main(){


		ricc::Dequeue<int> a;
		ricc::Dequeue<int> b{std::move(a)};



		return 0;
}

