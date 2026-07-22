#include <iostream>
#include "string.hpp"


int main(){

		
		ricc::String a("HI");
		

		std::cout << a.size() << std::endl;

		a.push_back('c');
		a.print();
		a.size();
		std::cout << a.size() << std::endl;
		std::cout << a.get_capactiy() << std::endl;
		
		ricc::String c("123456789012345678901234567890");
		c.push_back('f');
		c.print();
		std::cout << c.size() << std::endl;
		std::cout << c.get_capactiy() << std::endl;


		return 0;
}
