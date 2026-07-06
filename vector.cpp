#include <vector.hpp>
#include <assert.h>
#include <iostream>

int main(){

		ricc::vector<int> v1;

		assert(v1.size() == 0);
		
		v1.push_back(1);

		assert(v1.size() == 1);


}
