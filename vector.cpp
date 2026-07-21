#include "vector.hpp"

auto printVectorInt(const ricc::Vector<int>&  x){

		if (x.empty()){

				std::cout <<  "Empty Vector!" << std::endl;
		}

		for (std::size_t i{}; i < x.size(); ++i ){

				std::cout << x[i] << std::endl;
		}
}

auto printVectorString(const ricc::Vector<std::string>&  x){

		if (x.empty()){

				std::cout <<  "Empty Vector!" << std::endl;
		}

		for (std::size_t i{}; i < x.size(); ++i ){

				std::cout << x[i] << std::endl;
		}
}

int main(){

		ricc::Vector<int> a;
		ricc::Vector<int>b (std::move(a));
		b = std::move(a);

		return 0;


}
