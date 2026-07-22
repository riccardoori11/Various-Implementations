#include <iostream>
#include "string.hpp"


int main(){

		
		ricc::String a("Hello!12ewu18ur2098328t923");
		ricc::String b("hi913120rui2390uj3gj3r08ug903u03490ug0934u");

		a = b;
		a.print();

		std::cout << a.size() << std::endl;
		std::cout << b.size() << std::endl;
		
		b.print();


		

		return 0;
}
