#include <iostream>
#include <assert.h>
#include "unique_ptr.hpp"
#include <memory>



int main(){
		
		ricc::unique_ptr<int> p1(new int(10));			
		ricc::unique_ptr<int> p2;
		assert(p1);
		assert(!p2);
		
		int* x = new int(10);
		ricc::unique_ptr<int> ptr(x);
		int* rawptr = ptr.get();
		assert(*rawptr == 10);	
		assert(rawptr == x);

		ricc::unique_ptr<int> p3(new int(10));
		p3.reset(new int(20));
		assert(*p3 == 20);
		
		ricc::unique_ptr<int> p5(new int(10));
		ricc::unique_ptr<int> p6(new int(10));
		
		ricc::unique_ptr<int> p0;
		assert(p5 == p5);
		assert(p0 == nullptr);


		return 0;

}
