#include <any.hpp>
#include <cassert>
#include <iostream>

int main(){

		ricc::any a1(1.23);
		assert(a1.any_cast<double>() == 1.23);

		ricc::any a2(a1);
		assert(a2.any_cast<double>() == 1.23);

		ricc::any a3(std::move(a1));
		assert(a3.any_cast<double>() == 1.23);

		assert(a3.has_value() == true);

		ricc::any a4{};
		assert(a4.has_value() == false);

		{
		ricc::any a5{ricc::Tracker(4)};
		std::cout << ricc::Tracker::alive << std::endl;
		assert(ricc::Tracker::alive == 1);
		assert(ricc::Tracker::dead == 1);
		}
		assert(ricc::Tracker::alive == 0);
		assert(ricc::Tracker::dead == 2);

		return 0;
}
