#include <any.hpp>

int main(){

		ricc::any a1(1.23);
		std::cout << "a1 is double: " << a1.any_cast<double>() << std::endl;

		ricc::any a2(a1);
		std::cout << "a2 is double: " << a2.any_cast<double>() << std::endl;

		ricc::any a3(std::move(a1));
		std::cout << "a3 is double: " << a3.any_cast<double>() << std::endl;

		return 0;
}
