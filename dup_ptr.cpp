#include "dup_ptr.hpp"
#include <iostream>


int main(){

		int x = 4;
		int *p = &x;

		ricc::dup_ptr<int> p1{p};

		ricc::dup_ptr<int> p2{p1};

		std::cout << *p1 << std::endl;
		std::cout << *p2 << std::endl;

		return 0;
}
