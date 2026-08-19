#include <any.hpp>

int main(){

		ricc::any a1(1.23);
		std::cout << "a1 is float: " << a1.any_cast<double>() << std::endl;

		ricc::any a2(a1);
		std::cout << "a1 is float: " << a2.any_cast<double>() << std::endl;

		return 0;
}
