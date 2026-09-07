#include <iostream>
#include "hive.hpp"

int main(){

		ricc::hive<int> a{};

		a.insert(1);


		std::cout << a.size() << std::endl;

		return 0;
}
