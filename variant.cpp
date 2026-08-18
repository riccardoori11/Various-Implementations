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

		assert(ricc::holds_alternative<int>(value) == 0);
		assert(ricc::holds_alternative<double>(value) == 1);

		ricc::variant<int, float> v1 = 12;
		assert(ricc::get<int>(v1) == 12);


		ricc::variant<int,float> v2{v1};

		ricc::variant<int,float> v3{std::move(v1)};

		assert(ricc::get<int>(v3) == 12);
		assert(ricc::holds_alternative<int>(v3) == 1);
		assert(ricc::holds_alternative<float>(v3) == 0);

		return 0;
}
