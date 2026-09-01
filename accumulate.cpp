#include "accumulate.hpp"
#include "vector.hpp"

int main(){

		ricc::Vector<int> a{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22};

		auto rst = ricc::parallel_accumulate(a.begin(),a.end(),0);

		std::cout << rst << std::endl;

		return 0;
}
