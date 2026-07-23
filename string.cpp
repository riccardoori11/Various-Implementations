#include <iostream>
#include "string.hpp"


int main(){

		ricc::String a;
		a.assign(4, '=');

		a.print();

		std::cout << sizeof(a) << std::endl;

		return 0;
}
