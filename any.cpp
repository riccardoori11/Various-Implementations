#include <any.hpp>

int main(){

		auto a1 = ricc::any(12);
		std::cout << "a1 is int:" << a1.any_cast<int>() << std::endl;

		return 0;
}
