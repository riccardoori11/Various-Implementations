#include <iostream>
#include <assert.h>
#include <string>
#include <variant>
#include <variant.hpp>

int main(){

		std::variant<int,float> v;
		v = 12;

		assert(std::get<int>(v) == 12);

		ricc::variant<int, double> value{3.1};

		assert(ricc::holds_alternative<int>(value) == 0);
		assert(ricc::holds_alternative<double>(value) == 1);

		ricc::variant<int, float> v1 = 12;
		assert(ricc::get<int>(v1) == 12);

		std::cout << ricc::get<int>(v1) << std::endl;

		return 0;
}
