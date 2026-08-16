#include <iostream>
#include <assert.h>
#include "string.hpp"



int main(){
/*
		ricc::String a;
		a.assign(4, '=');

		a.print();

		std::cout << sizeof(a) << std::endl;
*/

		ricc::String a;
		a.pushBack('u');
		ricc::String b{a};
		assert(a[0] == b[0]);

		a.pushBack('4');

		b = a;
		assert(a[1] == b[1]);

		ricc::String c(std::move(a));
		assert(c[0] == b[0]);

		ricc::String d;
		d = std::move(b);
		assert(c[0] == d[0]);

		ricc::String s; 
		s.pushBack('H');
		s.pushBack('e');
		s.pushBack('l');
		s.pushBack('l');
		s.pushBack('o');

		std::cout << s << std::endl;


		return 0;
		
}
