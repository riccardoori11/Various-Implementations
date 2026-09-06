#include "array.hpp"

int main(){

		ricc::Array<int, 5> a {1,2,3};

		ricc::Array<int , 5> b{4};

		a = b;

		ricc::Array<int, 5> c {std::move(a)};
		ricc::Array<int, 5> d{9,9};

		c = d;

		std::cout << c[0] << std::endl;


		return 0;
}
