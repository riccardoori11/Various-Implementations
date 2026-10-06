#include <iostream>
#include "hive.hpp"

int main(){

		ricc::Fixed_sized_hive<int> a;

		auto first_it = a.insert(5);

		auto second_it = a.insert(30);

		for (int val: a){
				std::cout << val << std::endl;
		}

		a.erase(first_it);

		std::cout << std::endl;

		for (int val: a){
				std::cout << val << std::endl;
		}

		return 0;
}
