#include "unique_ptr.hpp"
#include <iostream>

int main(){

		ricc::unique_ptr<int> h(new int(10));

		std::cout << *h << std::endl;

		ricc::unique_ptr<int> p(std::move(h));

		std::cout << *p << std::endl;

		ricc::unique_ptr<int> ptr;
		ptr = std::move(p);
		std::cout << *ptr << std::endl;

		return 0;
}
