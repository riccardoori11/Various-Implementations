#include <iostream>
#include <assert.h>
#include "unique_ptr.hpp"
#include <memory>
#include <type_traits>

int main(){
		
					
		ricc::unique_ptr<int> p2 ;
		ricc::unique_ptr<int> p1 = new int(10);	
		
		
		assert(*p1.get() == 10);
		int *x = p1.release();
		assert(x != nullptr);
		assert(p1 == nullptr);
		
		assert(std::is_move_assignable_v<ricc::unique_ptr<int>>);
		assert(std::is_move_constructible_v<ricc::unique_ptr<int>>);	
		assert(!std::is_copy_constructible_v<ricc::unique_ptr<int>>);
		assert(!std::is_copy_assignable_v<ricc::unique_ptr<int>>);

		ricc::unique_ptr<int>p3 = new int(10);	
		p3.reset(new int (20));
		assert(*p3 == 20);
			


		return 0;

}
