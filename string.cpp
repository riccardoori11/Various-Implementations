#include <iostream>
#include "string.hpp"


int main(){

		
		ricc::String a("Hello!");
		ricc::String b("1234567890123456789012345678901234567890");

		ricc::String c(a);
		ricc::String d;
		d = std::move(c);
		d.print();
		d = std::move(b);
		d.print();


		return 0;
}
