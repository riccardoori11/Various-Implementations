#include <iostream>
#include <assert.h>
#include <string>
#include <variant>
#include <variant.hpp>

int main(){

		std::variant<int,float> v;
		v = 12;

		assert(std::get<int>(v) == 12);

		ricc::variant<double, int> value{3.1};


		return 0;
}
