#include "unique_ptr.hpp"
#include <assert.h>
#include <iostream>

int main(){

		ricc::unique_ptr<int> h(new int(10));

		assert(*h == 10);

		ricc::unique_ptr<int> p(std::move(h));

		assert(*p == 10);

		ricc::unique_ptr<int> ptr;
		ptr = std::move(p);
		assert(*ptr == 10);

		ricc::unique_ptr<int> null;
		assert(null == nullptr);

		ricc::unique_ptr<int> up1(new int(19));
		ricc::unique_ptr<int> up2(new int(10));

		up1.swap(up2);

		std::cout << *up1 << std::endl;
		
		return 0;
}
