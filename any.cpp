#include <any.hpp>

int main(){

		ricc::any a1(1);
		std::cout << "a1 is int: " << a1.any_cast<int>() << std::endl;

		std::string hi{"Hello World!"};
		a1 = hi;
		std::cout << "a1 is string: " << a1.any_cast<std::string>() << std::endl;

		return 0;
}
